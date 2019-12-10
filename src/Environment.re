open Node.Child_process;

let exec = (cmd, folder: VsCode.Workspace.Folder.t) => {
  let opts = option(~cwd=folder.uri.fsPath, ());
  execSync(cmd, opts);
};

let existsMerlinLspInEsyEnv = folder =>
  try({
    let esyEnv = exec("esy command-env", folder);
    exec(
      {j|eval $esyEnv && command -v ocamlmerlin-lsp|j},
      folder,
    )
    |> ignore;
    true;
  }) {
  | _ => false
  };

let existsMerlinLspGlobal = folder =>
  try(
    {
      exec("command -v ocamlmerlin-lsp", folder) |> ignore;
      true;
    }
  ) {
  | _ => false
  };

let serverOptionsForFolder = (_, folder) =>
  if (existsMerlinLspInEsyEnv(folder)) {
    let command = Node.Process.process##platform === "win32" ? "esy.cmd" : "esy";
    let args = [|"exec-command", "--include-current-env", "ocamlmerlin-lsp"|];
    Some(
      VsCode.{
        command,
        args,
        options: None,
      },
    );
  } else if (existsMerlinLspGlobal(folder)) {
    Some(VsCode.{command: "ocamlmerlin-lsp", args: [||], options: None});
  } else {
    None;
  };
