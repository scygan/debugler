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

//#include <DGLNet/gl-serialized.h>
#include <utility>

#include <vector>
#include <memory>
#include <DGLCommon/def.h>
#include <DGLNet/protocol/entrypoint.h>

class ActionBase;
extern std::shared_ptr<ActionBase> g_Actions[NUM_ENTRYPOINTS];

class ActionBase {
   public:
    virtual ~ActionBase() {}

    /**
     * Default, empty Pre() action. Subclasses may want to reimplement this
     */
    virtual RetValue Pre(const CalledEntryPoint&);

    /**
     * Default, empty Post() action. Subclasses may want to reimplement this
     */
    virtual void Post(const CalledEntryPoint&,
                      const RetValue& ret = RetValue());

    template <typename SpecificActionType>
    static void SetNext(Entrypoint entryp) {
        std::shared_ptr<ActionBase> prev = g_Actions[entryp];
        g_Actions[entryp] =
                std::shared_ptr<ActionBase>(new SpecificActionType());
        g_Actions[entryp]->SetPrev(prev);
    }

   protected:
    /**
     * Call previous action in Chain of Dependency
     */
    RetValue PrevPre(const CalledEntryPoint&);

    /**
     * Call previous action in Chain of Dependency
     */
    void PrevPost(const CalledEntryPoint&, const RetValue& ret);

   private:
    void SetPrev(const std::shared_ptr<ActionBase>& prev);
    std::shared_ptr<ActionBase> m_PrevAction;
};

class DefaultAction : public ActionBase {
    virtual RetValue Pre(const CalledEntryPoint&);
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class GLGetErrorAction : public ActionBase {
    virtual RetValue Pre(const CalledEntryPoint&);
};

class GetProcAddressAction : public ActionBase {
    virtual RetValue Pre(const CalledEntryPoint&);
};

#ifdef WA_ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID
class SurfaceAction : public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};
#endif

class ContextAction : public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class DebugContextAction : public ActionBase {
    virtual RetValue Pre(const CalledEntryPoint&);
    static bool anyContextPresent;
};

class TextureAction : public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class TextureFormatAction : public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class BufferAction : public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class ProgramAction : public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class ShaderAction : public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class ImmediateModeAction : public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class FBOAction : public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class DebugOutputCallback : public ActionBase {
    virtual RetValue Pre(const CalledEntryPoint&);
};

template <typename Action>
void SetAllActions() {
    for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
        g_Actions[i] = std::shared_ptr<ActionBase>(new Action());
    }
}
