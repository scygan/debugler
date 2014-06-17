/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#ifndef ACTIONS_H
#define ACTIONS_H

#include <utility>

#include <vector>
#include <memory>
#include <DGLCommon/def.h>
#include <DGLCommon/wa.h>
#include <DGLNet/protocol/entrypoint.h>

class ActionManager;

namespace actions {

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

    void SetPrev(const std::shared_ptr<ActionBase>& prev);

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
    std::shared_ptr<ActionBase> m_PrevAction;
};

class DefaultAction : public ActionBase {
    static void Register(ActionManager& mgr);
    virtual RetValue Pre(const CalledEntryPoint&);
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class ErrorAwareGLAction: public ActionBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret) final;

    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret) = 0;
};

class GLGetErrorAction : public ActionBase {
public:
    static void Register(ActionManager& mgr);
private:
    virtual RetValue Pre(const CalledEntryPoint&);
};

class GetProcAddressAction : public ActionBase {
public:
    static void Register(ActionManager& mgr);
private:
    virtual RetValue Pre(const CalledEntryPoint&);
};

#if DGL_HAVE_WA(ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID)
class SurfaceAction : public ActionBase {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};
#endif

class ContextAction : public ActionBase {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class DebugContextAction : public ActionBase {
public:
    static void Register(ActionManager& mgr);
private:
    virtual RetValue Pre(const CalledEntryPoint&);
    static bool anyContextPresent;
};

class TextureAction : public ErrorAwareGLAction {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret);
};

class TextureFormatAction : public ErrorAwareGLAction {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret);
};

class BufferAction : public ErrorAwareGLAction {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret);
};

class ProgramAction : public ErrorAwareGLAction {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret);
};

class ProgramPipelineAction : public ErrorAwareGLAction {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret);
};

class ShaderAction : public ErrorAwareGLAction {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret);
};

class ImmediateModeAction : public ErrorAwareGLAction {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret);
};

class FBOAction : public ErrorAwareGLAction {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret);
};

class RenderbufferAction : public ErrorAwareGLAction {
public:
    static void Register(ActionManager& mgr);
private:
    virtual void NoGLErrorPost(const CalledEntryPoint&, const RetValue& ret);
};

class DebugOutputCallback : public ActionBase {
public:
    static void Register(ActionManager& mgr);
private:
    virtual RetValue Pre(const CalledEntryPoint&);
};


} // namespace actions

#endif