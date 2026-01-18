/** @type {import('eslint').Rule.RuleModule} */
export default {
  meta: {
    type: 'problem',
    docs: {
      description: 'Enforce OnPush change detection strategy in components',
    },
    messages: {
      missingOnPush: 'Component must use ChangeDetectionStrategy.OnPush',
    },
    schema: [],
  },
  create(context) {
    return {
      Decorator(node) {
        if (node.expression.callee?.name !== 'Component') return;

        const classNode = node.parent;
        if (
          classNode?.type === 'ClassDeclaration' &&
          classNode.id?.name.startsWith('Stub')
        ) {
          return;
        }

        const arg = node.expression.arguments[0];
        if (!arg || arg.type !== 'ObjectExpression') return;

        const changeDetection = arg.properties.find(
          (prop) => prop.key?.name === 'changeDetection'
        );

        if (!changeDetection) {
          context.report({
            node,
            messageId: 'missingOnPush',
          });
          return;
        }

        const value = changeDetection.value;
        const isOnPush =
          value.property?.name === 'OnPush' ||
          (value.type === 'Identifier' && value.name === 'OnPush');

        if (!isOnPush) {
          context.report({
            node,
            messageId: 'missingOnPush',
          });
        }
      },
    };
  },
};
