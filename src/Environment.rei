open VsCode;

let serverOptionsForFolder:
  (Workspace.textDocumentEvent, Workspace.Folder.t) => option(serverOptions);
