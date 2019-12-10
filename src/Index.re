open VsCode;
/**
  * Formatting https://github.com/microsoft/vscode-languageserver-node/blob/master/client/src/client.ts#L477-L478
  https://github.com/rusty-key/vscode-ocaml-reason-format/blob/5a87ba8705877ab527349c0c7e0154b3897d264a/src/extension.ts
  */;

let activate = (context: VsCode.ExtensionContext.t) => {
  MultiWorkspace.start(
    ~context, ~commands=[||], ~createClient=(document, folder) => {
    switch (Environment.serverOptionsForFolder(document, folder)) {
    | Some(serverOptions) =>
      Some(
        LanguageClient.make(
          ~id="vscode-reason",
          ~name="Reason/OCaml Language Server Client for VSCode",
          ~serverOptions,
          ~clientOptions=None,
        ),
      )
    | None => None
    }
  });
};

