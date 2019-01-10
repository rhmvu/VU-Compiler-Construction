from util import ASTTransformer
from ast import Type, Operator, VarDef, ArrayDef, Assignment, Modification, \
        If, Block, VarUse, BinaryOp, IntConst, Return, While


class Desugarer(ASTTransformer):
    def __init__(self):
        self.varcache_stack = [{}]

    def makevar(self, name):
        # Generate a variable starting with an underscore (which is not allowed
        # in the language itself, so should be unique). To make the variable
        # unique, add a counter value if there are multiple generated variables
        # of the same name within the current scope.
        # A variable can be tagged as 'ssa' which means it is only assigned once
        # at its definition.
        name = '_' + name
        varcache = self.varcache_stack[-1]
        occurrences = varcache.setdefault(name, 0)
        varcache[name] += 1
        return name if not occurrences else name + str(occurrences + 1)

    def visitFunDef(self, node):
        self.varcache_stack.append({})
        self.visit_children(node)
        self.varcache_stack.pop()

    def visitModification(self, m):
        # from: lhs op= rhs
        # to:   lhs = lhs op rhs
        self.visit_children(m)
        return Assignment(m.ref, BinaryOp(m.ref, m.op, m.value)).at(m)

    def visitFor(self, node):
        # from: for(int node.name = node.expr1 to node.expr2) { node.body }
        # to:
        # Block wrapper {
        # Assignment: int node.name = node.expr1;
        # While type:  while ( expression: node.name < node.expr2){
        # new block:
        # Block:  node.body;
        # : node.name = node.name + 1
        # }
        self.visit_children(node)

        if isinstance(node.ref, str):
            ref = VarUse(node.ref)
        initial_assignment = Assignment(ref, node.expr1).at(node)
        incrementation = Assignment(ref, BinaryOp(ref, Operator('+'), IntConst(1))).at(node)
        while_body = Block([node.body, incrementation]).at(node)
        while_condition = BinaryOp(ref, Operator("<"), node.expr2).at(node)
        while_statement = While(while_condition, while_body).at(node)
        return Block([initial_assignment, while_statement]).at(node)


