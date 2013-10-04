// create a combo box for language definition file selections
languageComboBox = new srchiliteqt::LanguageComboBox();
languageToolBar = addToolBar(tr("Language"));
languageToolBar->addWidget(languageComboBox);
// retrieve the current language by the text editor highlighter
languageComboBox->setCurrentLanguage
    (textEdit->getHighlighter()->getLangFile());
// and connect it to our text editor
textEdit->connectLanguageComboBox(languageComboBox);

styleComboBox = new srchiliteqt::StyleComboBox;
styleToolBar = addToolBar(tr("Style"));
styleToolBar->addWidget(styleComboBox);
styleComboBox->setCurrentStyle("default.style");
// each time we insert something it will be resized
styleComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
textEdit->connectStyleComboBox(styleComboBox);
