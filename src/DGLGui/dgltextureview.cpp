/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "dgltextureview.h"

#include <DGLCommon/def.h>

DGLTextureViewItem::DGLTextureViewItem(dglnet::ContextObjectName name,
                                       DGLResourceManager* resManager,
                                       QWidget* parrent)
        : DGLTabbedViewItem(name, parrent),
          m_CurrentLevel(0),
          m_CurrentLayer(0),
          m_CurrentFace(0) {
    m_Ui.setupUi(this);

    m_PixelRectangleScene = new DGLPixelRectangleScene();
    m_Ui.m_PixelRectangleView->setScene(m_PixelRectangleScene);

    m_Listener = resManager->createListener(
            name, dglnet::DGLResource::ObjectType::Texture);
    m_Listener->setParent(this);

    m_Ui.horizontalSlider_LOD->setDisabled(true);

    CONNASSERT(m_Ui.horizontalSlider_Layer, SIGNAL(sliderMoved(int)), this,
        SLOT(layerSliderMoved(int)));

    CONNASSERT(m_Ui.horizontalSlider_LOD, SIGNAL(sliderMoved(int)), this,
               SLOT(levelSliderMoved(int)));
    CONNASSERT(m_Ui.comboBoxCM, SIGNAL(currentIndexChanged(int)), this,
               SLOT(faceComboChanged(int)));

    CONNASSERT(m_Listener, SIGNAL(update(const dglnet::DGLResource&)), this,
               SLOT(update(const dglnet::DGLResource&)));
    CONNASSERT(m_Listener, SIGNAL(error(const std::string&)), this,
               SLOT(error(const std::string&)));

    m_Ui.labelCM->hide();
    m_Ui.comboBoxCM->hide();

    m_Ui.label_TextureLayer->hide();
    m_Ui.horizontalSlider_Layer->hide();
}

void DGLTextureViewItem::error(const std::string& message) {
    m_PixelRectangleScene->setText(message);
    m_Ui.horizontalSlider_LOD->setDisabled(true);
    m_Ui.horizontalSlider_Layer->setDisabled(true);
    m_Ui.m_PixelRectangleView->updateFormatSizeInfo(NULL);
}

void DGLTextureViewItem::update(const dglnet::DGLResource& res) {
    const dglnet::resource::DGLResourceTexture* resource =
            dynamic_cast<const dglnet::resource::DGLResourceTexture*>(&res);

    m_FacesLevelsLayers = resource->m_FacesLevelsLayers;

    m_CurrentFace = std::min(
        m_CurrentFace, static_cast<uint>(m_FacesLevelsLayers.size() - 1));

    if (m_FacesLevelsLayers.size() && m_FacesLevelsLayers[0].size()) {

        if (m_FacesLevelsLayers.size() > 1) {
            // texture has faces
            m_Ui.labelCM->show();
            m_Ui.comboBoxCM->show();
        } else {
            m_Ui.labelCM->hide();
            m_Ui.comboBoxCM->hide();
        }

        m_Ui.comboBoxCM->setCurrentIndex(m_CurrentFace);


        m_CurrentLevel = std::min(
            m_CurrentFace, static_cast<uint>(m_FacesLevelsLayers[m_CurrentFace].size() - 1));

        m_CurrentLayer = std::min(
            m_CurrentFace, static_cast<uint>(m_FacesLevelsLayers[m_CurrentFace][m_CurrentLevel].size() - 1));


        m_Ui.horizontalSlider_LOD->setRange(
            0, static_cast<int>(m_FacesLevelsLayers[m_CurrentFace].size() - 1));
        m_Ui.horizontalSlider_LOD->setEnabled(true);


        if (resource->m_Target == GL_TEXTURE_3D || resource->m_Target == GL_TEXTURE_2D_ARRAY) {
            
            m_Ui.label_TextureLayer->show();
            m_Ui.horizontalSlider_Layer->show();
        } else {
            m_Ui.label_TextureLayer->hide();
            m_Ui.horizontalSlider_Layer->hide();
        }

        m_Ui.horizontalSlider_Layer->setRange(
            0, static_cast<int>(m_FacesLevelsLayers[m_CurrentFace][m_CurrentLevel].size() - 1));

        internalUpdate();

    } else {
        error("Texture is empty");
    }
}

void DGLTextureViewItem::faceComboChanged(int value) {
    uint uvalue = value;
    if (uvalue != m_CurrentFace && uvalue < static_cast<uint>(m_FacesLevelsLayers.size())) {
        m_CurrentFace = uvalue;
        internalUpdate();
    }
}

void DGLTextureViewItem::layerSliderMoved(int value) {
    uint uvalue = value;
    if (uvalue != m_CurrentLayer &&
        uvalue < static_cast<uint>(m_FacesLevelsLayers[m_CurrentFace][m_CurrentLevel].size())) {
            m_CurrentLayer = uvalue;
            internalUpdate();
    }
}

void DGLTextureViewItem::levelSliderMoved(int value) {
    uint uvalue = value;
    if (uvalue != m_CurrentLevel &&
        uvalue < static_cast<uint>(m_FacesLevelsLayers[m_CurrentFace].size())) {

        m_CurrentLevel = uvalue;

        m_Ui.horizontalSlider_Layer->setRange(
            0, static_cast<int>(m_FacesLevelsLayers[m_CurrentFace][m_CurrentLevel].size() - 1));

        m_CurrentLayer = std::min(
            m_CurrentFace, static_cast<uint>(m_FacesLevelsLayers[m_CurrentFace][m_CurrentLevel].size() - 1));

        internalUpdate();
    }
   
}

void DGLTextureViewItem::internalUpdate() {

    // validate m_CurrentFace
    if (m_CurrentFace >= static_cast<uint>(m_FacesLevelsLayers.size())) {
        m_PixelRectangleScene->setText("Selected texture face does not exists");
        return;
    }

    // validate m_CurrentLevel
    if (m_CurrentLevel >= static_cast<uint>(m_FacesLevelsLayers[m_CurrentFace].size())) {
        m_PixelRectangleScene->setText(
            "Selected texture level does not exists");
        return;
    }

    //validate m_CurrentLayer
    if (m_CurrentLayer >= static_cast<uint>(m_FacesLevelsLayers[m_CurrentFace][m_CurrentLevel].size())) {
        m_PixelRectangleScene->setText("Selected texture layer does not exists");
        return;
    }    

    m_PixelRectangleScene->setPixelRectangle(
            *m_FacesLevelsLayers[m_CurrentFace][m_CurrentLevel][m_CurrentLayer].get());
    m_Ui.m_PixelRectangleView->updateFormatSizeInfo(
            m_FacesLevelsLayers[m_CurrentFace][m_CurrentLevel][m_CurrentLayer].get());
}

DGLTextureView::DGLTextureView(QWidget* parrent, DglController* controller)
        : DGLTabbedView(parrent, controller) {
    setupNames("Textures", "DGLTextureView");

    // inbound
    CONNASSERT(controller->getViewRouter(),
               SIGNAL(showTexture(opaque_id_t, gl_t)), this,
               SLOT(showTexture(opaque_id_t, gl_t)));
}

void DGLTextureView::showTexture(opaque_id_t ctx, gl_t name) {
    ensureTabDisplayed(ctx, name);
}

DGLTabbedViewItem* DGLTextureView::createTab(
        const dglnet::ContextObjectName& id) {
    return new DGLTextureViewItem(id, m_Controller->getResourceManager(), this);
}

QString DGLTextureView::getTabName(gl_t id, gl_t /*target*/) {
    return QString("Texture ") + QString::number(id);
}
