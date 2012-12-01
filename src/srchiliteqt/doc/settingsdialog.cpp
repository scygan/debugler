srchiliteqt::SourceHighlightSettingsDialog dialog(this);
dialog.setSourceHighlightDataDirPath(sourceHighlightDataDir);

if (dialog.exec() == QDialog::Accepted) {
  if (sourceHighlightDataDir != dialog.getSourceHighlightDataDirPath()) {
    sourceHighlightDataDir = dialog.getSourceHighlightDataDirPath();
    srchilite::Settings::setGlobalDataDir
      (sourceHighlightDataDir.toStdString());
    reloadComboBoxes();
  }
}


if (!srchilite::Settings::checkSettings()) {
  QMessageBox::critical(this, tr("qeditexample"),
    tr("Source-highlight settings are wrong!\n\
       Please configure it correctly"));
} else {
  // make sure to reload the source-highlight global instances
  srchilite::Instances::reload();
}
