open Llvm

exception Error of string

let context = global_context ()
let the_module = create_module context "Rhine JIT"
let builder = builder context
let named_values:(string, llvalue) Hashtbl.t = Hashtbl.create 10
let i64_type = i64_type context
let i1_type = i1_type context
let double_type = double_type context
let void_type = void_type context

let int_of_bool = function true -> 1 | false -> 0

let extract_args s = match s with
    Ast.DottedPair(s1, s2) ->
    (match (s1, s2) with
       (Ast.Atom n, Ast.DottedPair(Ast.Atom m, Ast.Atom(Ast.Nil))) -> (n, m)
     | _ -> raise (Error "Expected two atoms"))
  | _ -> raise (Error "Expected sexp")

let codegen_atom = function
    Ast.Int n -> const_int i64_type n
  | Ast.Bool n -> const_int i1_type (int_of_bool n)
  | Ast.Double n -> const_float double_type n
  | Ast.Nil -> const_null i1_type
  | Ast.Symbol n -> raise (Error "Can't codegen_atom a symbol")

let codegen_operator op s2 =
  let lhs_val = codegen_atom (fst (extract_args s2)) in
  let rhs_val = codegen_atom (snd (extract_args s2)) in
  match op with
    "+" -> build_add lhs_val rhs_val "addtmp" builder
  | "-" -> build_sub lhs_val rhs_val "subtmp" builder
  | "*" -> build_mul lhs_val rhs_val "multmp" builder
  | _ -> raise (Error "Unknown operator")

let rec codegen_sexpr s = match s with
    Ast.Atom n -> codegen_atom n
  | Ast.DottedPair(s1, s2) ->
     begin match s1 with
             Ast.Atom a ->
             begin match a with
                     Ast.Symbol s -> codegen_operator s s2
                   | _ -> raise (Error "Expected function call")
             end
           | _ -> raise (Error "Sexpr parser broken!")
     end

let codegen_proto = function
  | Ast.Prototype (name, args) ->
      (* Make the function type: double(double,double) etc. *)
      let doubles = Array.make (Array.length args) double_type in
      let ft = function_type double_type doubles in
      let f =
        match lookup_function name the_module with
        | None -> declare_function name ft the_module

        (* If 'f' conflicted, there was already something named 'name'. If it
         * has a body, don't allow redefinition or reextern. *)
        | Some f ->
            (* If 'f' already has a body, reject this. *)
            if block_begin f <> At_end f then
              raise (Error "redefinition of function");

            (* If 'f' took a different number of arguments, reject. *)
            if element_type (type_of f) <> ft then
              raise (Error "redefinition of function with different # args");
            f
      in

      (* Set names for all arguments. *)
      Array.iteri (fun i a ->
        let n = args.(i) in
        set_value_name n a;
        Hashtbl.add named_values n a;
      ) (params f);
      f

let codegen_func = function
  | Ast.Function (proto, body) ->
      Hashtbl.clear named_values;
      let the_function = codegen_proto proto in

      (* Create a new basic block to start insertion into. *)
      let bb = append_block context "entry" the_function in
      position_at_end bb builder;

      try
        let ret_val = codegen_sexpr body in

        (* Finish off the function. *)
        let _ = build_ret ret_val builder in

        the_function
      with e ->
        delete_function the_function;
        raise e