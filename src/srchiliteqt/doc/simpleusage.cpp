QTextEdit *editor = new QTextEdit;
srchiliteqt::Qt4SyntaxHighlighter *highlighter =
  new srchiliteqt::Qt4SyntaxHighlighter(editor->document());
highlighter->init("java.lang");
