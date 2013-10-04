ColorDialog dialog(textEdit->getHighlighter(), this);

if (dialog.exec() == QDialog::Accepted) {
  dialog.syncFormatters();
  textEdit->getHighlighter()->rehighlight();

  // updating text editor colors is still up to you
  textEdit->changeColors
    (textEdit->getHighlighter()->getForegroundColor(),
     textEdit->getHighlighter()->getBackgroundColor());
}
