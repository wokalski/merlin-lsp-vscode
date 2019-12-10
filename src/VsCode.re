type memento;

type executableOptions;

/* https://github.com/microsoft/vscode-languageserver-node/blob/6233a284e85562c2ccdaa0b2d8dfd6a8f41ec2e0/client/src/main.ts#L51 */
type serverOptions = {
  command: string,
  args: array(string),
  options: option(executableOptions),
};

type documentSelectorItem = {
  scheme: string,
  language: string
};

type clientOptions = {
  documentSelector: array(documentSelectorItem)
};

module LanguageClient = {
  type t = {
    start: (. unit) => Js.Promise.t(unit),
    stop: (. unit) => unit,
  };
  [@bs.new] [@bs.module "vscode-languageclient"]
  external make:
    (
      ~id: string,
      ~name: string,
      ~serverOptions: serverOptions,
      ~clientOptions: option(clientOptions)
    ) =>
    t =
    "LanguageClient";
};

module Commands = {
  [@bs.module "vscode"] [@bs.scope "commands"]
  external get: (~filterInternal: bool) => Js.Promise.t(array(string)) =
    "getCommands";
  [@bs.module "vscode"] [@bs.scope "commands"]
  external register: (~command: string, ~handler: unit => unit) => unit =
    "registerCommand";
};

module Window = {
  [@bs.module "vscode"] [@bs.scope "window"]
  external showInformationMessage: string => unit = "showInformationMessage";
};

module ExtensionContext = {
  type disposable = {dispose: (. unit) => unit};

  type t = {
    extensionPath: string,
    globalState: memento,
    globalStoragePath: string,
    logPath: string,
    storagePath: option(string),
    /* subscriptions is an array of disposables. client is a disposable */
    subscriptions: array(disposable),
    workspaceState: memento,
    asAbsolutePath: string => string,
  };
};

module Workspace = {
  type uri = {
    scheme: string,
    fsPath: string,
  };
  type textDocumentEvent = {uri};
  module Folder = {
    type t = {uri};
    let key = f => f.uri.fsPath;
  };
  type workspaceFoldersChangeEvent = {
    added: array(Folder.t),
    removed: array(Folder.t),
  };

  [@bs.module "vscode"] [@bs.scope "workspace"]
  external onDidOpenTextDocument: (textDocumentEvent => unit) => unit =
    "onDidOpenTextDocument";
  [@bs.module "vscode"] [@bs.scope "workspace"]
  external getWorkspaceFolder: uri => option(Folder.t) = "getWorkspaceFolder";
  [@bs.module "vscode"] [@bs.scope "workspace"]
  external onDidChangeWorkspaceFolders:
    (workspaceFoldersChangeEvent => unit) => unit =
    "onDidChangeWorkspaceFolders";
  [@bs.module "vscode"] [@bs.scope "workspace"]
  external textDocuments: array(textDocumentEvent) = "textDocuments";
};

module MultiWorkspace = {
  module FoldersMap = Belt.HashMap.String;
  let start = (~context, ~commands, ~createClient) => {
    let workspaceFolders = FoldersMap.make(~hintSize=1);
    let onDidOpenTextDocument = (document: Workspace.textDocumentEvent) => {
      let uri = document.Workspace.uri;
      let folder = Workspace.getWorkspaceFolder(uri);
      switch (folder) {
      | Some(folder) when FoldersMap.has(workspaceFolders, folder.uri.fsPath) =>
        ()
      | Some(folder) =>
        switch (createClient(document, folder)) {
        | Some(client) =>
          FoldersMap.set(
            workspaceFolders,
            Workspace.Folder.key(folder),
            client,
          );
          client.LanguageClient.start(.)
          |> ignore;
        | None => ()
        }
      | None => ()
      };
    };
    Workspace.onDidOpenTextDocument(onDidOpenTextDocument);
    Workspace.textDocuments |> Js.Array.forEach(onDidOpenTextDocument);
    Workspace.onDidChangeWorkspaceFolders(event => {
      event.removed
      |> Js.Array.forEach(folder => {
           switch (
             FoldersMap.get(workspaceFolders, Workspace.Folder.key(folder))
           ) {
           | Some(client) =>
             FoldersMap.remove(
               workspaceFolders,
               Workspace.Folder.key(folder),
             );
             client.LanguageClient.stop(.);
           | None => ()
           }
         })
    });

    commands
    |> Js.Array.forEach(((cmd, handler)) => {
         Commands.register(~command=cmd, ~handler)
       });

    ExtensionContext.(
      Js.Array.push(
        {
          dispose:
            (.) => {
              FoldersMap.forEach(workspaceFolders, (_, client) => {
                client.LanguageClient.stop(.)
              });
              FoldersMap.clear(workspaceFolders);
            },
        },
        context.subscriptions,
      )
    ) |> ignore;
    ();
  };
};
