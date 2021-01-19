const parser = require("luaparse");
const fs = require("fs");
const _ = require("lodash");

let className = "ClassName";
let lineToComment = {};
let cppFunctions = [];

function indentBlock(s) {
  let lines = s.split("\n");
  for (let i = 0; i < lines.length; i++) {
    lines[i] = "  " + lines[i];
  }
  return lines.join("\n");
}

function handleFunctionDeclaration(block) {
  //console.log(block);
  let comment = lineToComment[block.loc.start.line - 1];
  let definition = block.identifier
    ? handleBlock(block.identifier)
    : "function";

  let parameters = [];
  for (let i = 0; i < block.parameters.length; i++) {
    let type = "TYPE";
    let param = handleBlock(block.parameters[i]);

    try {
      if (comment) {
        type = _.trim(comment.split(", ")[i]);
      }
    } catch (e) {}

    parameters.push(type + " " + param);
  }

  definition += "(" + parameters.join(", ") + ")";

  let cppFunction = {
    definition,
    body: handleBody(block.body),
    isLocal: block.isLocal,
  };

  cppFunctions.push(cppFunction);
}

function handleMemberExpression(block) {
  let base = handleBlock(block.base);
  let identifier = handleBlock(block.identifier);

  if (block.indexer == ".") {
    if (base == "self") {
      return identifier;
    } else {
      return base + "->" + identifier;
    }
  } else if (block.indexer == ":") {
    if (base == "self" || base == className) {
      return identifier;
    } else {
      return base + "->" + identifier;
    }
  } else {
    throw new Error("unhandled member expression: " + JSON.stringify(block));
  }
}

function handleIdentifier(block) {
  return block.name;
}

function handleReturnStatement(block) {
  let lines = [];
  for (let i = 0; i < block.arguments.length; i++) {
    let line = "return " + handleBlock(block.arguments[i]);
    if (i > 0) {
      line = "// " + line;
    }
    lines.push(line);
  }

  if (lines.length == 0) {
    return "return";
  }

  return lines.join("\n");
}

function handleLocalStatement(block) {
  let lines = [];
  let comment = lineToComment[block.loc.start.line - 1];

  for (
    let i = 0;
    i < Math.min(block.variables.length, block.init.length);
    i++
  ) {
    let type = "TYPE";

    try {
      if (comment) {
        type = _.trim(comment.split(", ")[i]);
      }
    } catch (e) {}

    let line = type + " " + handleBlock(block.variables[i]);
    line += " = ";
    line += handleBlock(block.init[i]);
    lines.push(line);
  }

  if (block.variables.length != block.init.length) {
    lines = ["// number of variables and initializers are not equal", ...lines];
  }

  return lines.join("\n");
}

function handleBooleanLiteral(block) {
  return block.value;
}

function handleNumericLiteral(block) {
  return block.value;
}

function handleStringLiteral(block) {
  return block.raw.replace(/\'/g, '"');
}

function handleNilLiteral(block) {
  return "null";
}

function handleBinaryExpression(block) {
  if (block.operator == "~=") {
    block.operator = "!=";
  }

  if (block.operator == "..") {
    block.operator = "+";
  }

  return (
    handleBlock(block.left) +
    " " +
    block.operator +
    " " +
    handleBlock(block.right)
  );
}

function handleCallExpression(block) {
  let args = [];

  for (let i = 0; i < block.arguments.length; i++) {
    args.push(handleBlock(block.arguments[i]));
  }

  return handleBlock(block.base) + "(" + args.join(", ") + ")";
}

function handleAssignmentStatement(block) {
  let lines = [];

  for (
    let i = 0;
    i < Math.min(block.variables.length, block.init.length);
    i++
  ) {
    let line = handleBlock(block.variables[i]);
    line += " = ";
    line += handleBlock(block.init[i]);
    lines.push(line);
  }

  if (block.variables.length != block.init.length) {
    lines = ["// number of variables and initializers are not equal", ...lines];
  }

  return lines.join("\n");
}

function handleIfStatement(block) {
  let lines = [];

  for (let i = 0; i < block.clauses.length; i++) {
    let clause = block.clauses[i];
    let line = "";
    switch (clause.type) {
      case "IfClause":
        line = "if (" + handleBlock(clause.condition) + ") {";
        break;
      case "ElseIfClause":
        line = " else if (" + handleBlock(clause.condition) + ") {";
        break;
      default:
        line = " else {";
    }

    line += "\n" + indentBlock(handleBody(clause.body));

    line += "\n}";

    lines.push(line);
  }

  return lines.join("");
}

function handleUnaryExpression(block) {
  if (block.operator == "not") {
    block.operator = "!";
  } else if (block.operator == "#") {
    return handleBlock(block.argument) + ".length";
  }

  return block.operator + handleBlock(block.argument);
}

function handleForNumericStatement(block) {
  let variable = handleBlock(block.variable);
  let start = handleBlock(block.start);
  let end = handleBlock(block.end);
  let step = block.step ? handleBlock(block.step) : "1";
  let body = indentBlock(handleBody(block.body));

  if (start == "1" && end.includes("length")) {
    start = "0";
  } else if (end == "1" && start.includes("length")) {
    end = "0";
  }

  let increment = variable + " += " + step;
  if (step == "1") {
    increment = variable + "++";
  }

  return (
    "for (" +
    variable +
    " = " +
    start +
    "; " +
    variable +
    " < " +
    end +
    "; " +
    increment +
    ") {\n" +
    body +
    "\n}"
  );
}

function handleIndexExpression(block) {
  return handleBlock(block.base) + "[" + handleBlock(block.index) + "]";
}

function handleCallStatement(block) {
  return handleBlock(block.expression);
}

function handleCallExpression(block) {
  let args = [];

  for (let i = 0; i < block.arguments.length; i++) {
    args.push(handleBlock(block.arguments[i]));
  }

  return handleBlock(block.base) + "(" + args.join(", ") + ")";
}

function handleLogicalExpression(block) {
  let operator;
  switch (block.operator) {
    case "and":
      operator = "&&";
      break;
    case "or":
      operator = "||";
      break;
    default:
      throw new Error("unknown operator: " + block.operator);
  }

  return (
    handleBlock(block.left) + " " + operator + " " + handleBlock(block.right)
  );
}

function handleTableConstructorExpression(block) {
  let isPoint = true;
  let isAllTableValue = true;

  if (block.fields.length == 0) {
    return "[]";
  }

  if (block.fields.length == 2) {
    for (let i = 0; i < block.fields.length; i++) {
      if (block.fields[i].type == "TableKeyString") {
        let key = handleBlock(block.fields[i].key);

        if (key != "x" && key != "y" && key != "dx" && key != "dy") {
          isPoint = false;
        }
      }
    }
  } else {
    isPoint = false;
  }

  for (let i = 0; i < block.fields.length; i++) {
    if (block.fields[i].type == "TableKeyString") {
      isAllTableValue = false;
    }
  }

  if (isPoint) {
    return (
      "Point(" +
      handleBlock(block.fields[0].value) +
      ", " +
      handleBlock(block.fields[1].value) +
      ")"
    );
  } else if (isAllTableValue) {
    let values = [];

    for (let i = 0; i < block.fields.length; i++) {
      values.push(handleBlock(block.fields[i].value));
    }

    return "[" + values.join(", ") + "]";
  } else {
    let rows = [];
    for (let i = 0; i < block.fields.length; i++) {
      rows.push(
        handleBlock(block.fields[i].key) +
          " = " +
          handleBlock(block.fields[i].value)
      );
    }

    return "{\n" + indentBlock(rows.join("\n")) + "\n}";
  }
}

function handleWhileStatement(block) {
  return (
    "while (" +
    handleBlock(block.condition) +
    ") {\n" +
    indentBlock(handleBody(block.body)) +
    "\n}"
  );
}

function handleBlock(block) {
  switch (block.type) {
    case "FunctionDeclaration":
      return handleFunctionDeclaration(block);
    case "MemberExpression":
      return handleMemberExpression(block);
    case "Identifier":
      return handleIdentifier(block);
    case "ReturnStatement":
      return handleReturnStatement(block);
    case "LocalStatement":
      return handleLocalStatement(block);
    case "BooleanLiteral":
      return handleBooleanLiteral(block);
    case "NumericLiteral":
      return handleNumericLiteral(block);
    case "StringLiteral":
      return handleStringLiteral(block);
    case "NilLiteral":
      return handleNilLiteral(block);
    case "BinaryExpression":
      return handleBinaryExpression(block);
    case "CallExpression":
      return handleCallExpression(block);
    case "AssignmentStatement":
      return handleAssignmentStatement(block);
    case "IfStatement":
      return handleIfStatement(block);
    case "UnaryExpression":
      return handleUnaryExpression(block);
    case "ForNumericStatement":
      return handleForNumericStatement(block);
    case "IndexExpression":
      return handleIndexExpression(block);
    case "CallStatement":
      return handleCallStatement(block);
    case "CallExpression":
      return handleCallExpression(block);
    case "LogicalExpression":
      return handleLogicalExpression(block);
    case "TableConstructorExpression":
      return handleTableConstructorExpression(block);
    case "WhileStatement":
      return handleWhileStatement(block);
    default:
      throw new Error("unhandled: " + block.type);
  }
}

function handleBody(body) {
  let lines = [];
  for (let i = 0; i < body.length; i++) {
    let line = handleBlock(body[i]);
    if (line) {
      lines.push(line);
    }
  }

  let result = lines.join("\n");
  let realLines = result.split("\n");

  for (let i = 0; i < realLines.length; i++) {
    let lastChar = realLines[i].charAt(realLines[i].length - 1);
    if (
      _.trim(realLines[i]).length > 0 &&
      lastChar != ";" &&
      lastChar != "{" &&
      lastChar != "}" &&
      !_.trim(realLines[i]).startsWith("//")
    ) {
      realLines[i] = realLines[i] + ";";
    }
  }

  return realLines.join("\n");
}

async function runAsync() {
  let file = fs.readFileSync("../scene-creator/tools/draw_data.lua", "utf-8");
  var ast = parser.parse(file, {
    locations: true,
  });

  //console.log(JSON.stringify(ast, null, 2));

  for (let i = 0; i < ast.comments.length; i++) {
    let comment = ast.comments[i];
    lineToComment[comment.loc.start.line] = _.trim(comment.value);
  }

  if (lineToComment[1]) {
    className = lineToComment[1];
  }

  handleBody(ast.body);

  let lines = [];
  for (let i = 0; i < cppFunctions.length; i++) {
    lines.push(
      "TYPE " +
        className +
        "::" +
        cppFunctions[i].definition +
        " {\n" +
        indentBlock(cppFunctions[i].body) +
        "\n}"
    );
  }

  let result = lines.join("\n\n");

  fs.writeFileSync("out.cpp", result);
}

runAsync().then(process.exit);
