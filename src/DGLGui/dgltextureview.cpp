#include "dgltextureview.h"
#include "dglgui.h"

DGLTextureViewItem::DGLTextureViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent), m_CurrentFace(0), m_CurrentLevel(0) {
    m_Ui.setupUi(this);
    
    m_PixelRectangleScene = new DGLPixelRectangleScene();
    m_PixelRectangleView = boost::make_shared<DGLPixelRectangleView>(this, m_PixelRectangleScene);
    m_PixelRectangleView->setMinimumSize(QSize(400, 320));
    m_Ui.verticalLayout->insertWidget(0, m_PixelRectangleView.get());

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeTexture);
    m_Listener->setParent(this);

    m_Ui.horizontalSlider_LOD->setDisabled(true);

    CONNASSERT(connect(m_Ui.horizontalSlider_LOD,SIGNAL(sliderMoved(int)),this,SLOT(levelSliderMoved(int))));
    CONNASSERT(connect(m_Ui.comboBoxCM,SIGNAL(currentIndexChanged(int)),this,SLOT(faceComboChanged(int))));

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));

    m_Ui.labelCM->hide();
    m_Ui.comboBoxCM->hide();
}

void DGLTextureViewItem::error(const std::string& message) {
    m_PixelRectangleScene->setText(message);
    m_Ui.horizontalSlider_LOD->setDisabled(true);
}

void DGLTextureViewItem::update(const DGLResource& res) {
    const DGLResourceTexture* resource = dynamic_cast<const DGLResourceTexture*>(&res);

    m_FacesLevels = resource->m_FacesLevels;

    if (m_FacesLevels.size()) {
        
        if (m_FacesLevels.size() > 1) {
            //texture has faces
            m_Ui.labelCM->show();
            m_Ui.comboBoxCM->show();

            m_CurrentFace = min(m_CurrentFace, (m_FacesLevels.size() - 1));

        } else {
            m_Ui.labelCM->hide();
            m_Ui.comboBoxCM->hide();
            m_CurrentFace = 0;
        }

        m_Ui.comboBoxCM->setCurrentIndex(m_CurrentFace);
        m_Ui.horizontalSlider_LOD->setRange(0, m_FacesLevels[m_CurrentFace].size() - 1);
        m_Ui.horizontalSlider_LOD->setEnabled(true);

        internalUpdate();

    } else {
        error("Texture empty");
    }
}

void DGLTextureViewItem::faceComboChanged(int value) {
    uint uvalue = value;
    if (uvalue != m_CurrentFace && uvalue < m_FacesLevels.size()) {
        m_CurrentFace = uvalue;
        internalUpdate();
    }
}

void DGLTextureViewItem::levelSliderMoved(int value) {
    uint uvalue = value;
    if (uvalue != m_CurrentLevel && uvalue < m_FacesLevels[m_CurrentFace].size()) {
        m_CurrentLevel = uvalue;
        internalUpdate();
    }
}

void DGLTextureViewItem::internalUpdate() {

    //validate m_CurrentFace
    if (m_CurrentFace >= m_FacesLevels.size()) {
         m_PixelRectangleScene->setText("Selected texture face does not exists");
         return;
    }

    //validate m_CurrentLevel
    if (m_CurrentLevel >= m_FacesLevels[m_CurrentFace].size()) {
        m_PixelRectangleScene->setText("Selected texture level does not exists");
        return;
    }
    
    m_PixelRectangleScene->setPixelRectangle(&m_FacesLevels[m_CurrentFace][m_CurrentLevel]);
}


DGLTextureView::DGLTextureView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Textures", "DGLTextureView");
   	
    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showTexture(uint)), this, SLOT(showTexture(uint))));
        
}

void DGLTextureView::showTexture(uint name) {
    ensureTabDisplayed(name);
}

DGLTabbedViewItem* DGLTextureView::createTab(uint id) {
    return new DGLTextureViewItem(id, m_ResourceManager, this);
}

QString DGLTextureView::getTabName(uint id, uint target) {
    return QString("Texture ") + QString::number(id);
}