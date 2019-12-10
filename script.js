const { execSync } = require("child_process");

const env = execSync(`esy command-env`);
execSync(`eval ${env} && command -v ocamlmerlin-lsp`);

