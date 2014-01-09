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

#ifndef DGLADBINTERFACE_H
#define DGLADBINTERFACE_H

#include <memory>
#include <string>
#include <vector>
#include <set>

#include <QRegExp>

#include "dglprocess.h"

class DGLAdbOutputFilter;

class DGLAdbCookie;

class DGLAdbHandler {
   public:
    virtual ~DGLAdbHandler();
    virtual void done(const std::vector<std::string>& data) = 0;
    virtual void failed(const std::string& reason) = 0;
    void refCookie(DGLAdbCookie*);
    void unrefCookie(DGLAdbCookie*);

   private:
    std::set<DGLAdbCookie*> m_RefCookies;
};

class DGLAdbCookie : public QObject {
    Q_OBJECT
   public:
    DGLAdbCookie(DGLAdbHandler* handler,
                 std::shared_ptr<DGLAdbOutputFilter> filter);
    virtual ~DGLAdbCookie();
    virtual void process() = 0;

   protected:
    void onDone(std::vector<std::string> data);
    void onFailed(std::string reason);

    void filterOutput(const std::vector<std::string>& lines);

    std::shared_ptr<DGLAdbOutputFilter> m_OutputFilter;

    DGLAdbHandler* m_Handler;
};

class DGLAdbCookieImpl : public DGLAdbCookie {
    Q_OBJECT
   public:
    DGLAdbCookieImpl(const std::string& adbPath,
                     const std::vector<std::string>& params,
                     DGLAdbHandler* handler,
                     std::shared_ptr<DGLAdbOutputFilter> filter);

    virtual void process() override;

   private
slots:
    void handleProcessError(QProcess::ProcessError);
    void handleProcessFinished(int code, QProcess::ExitStatus status);

   private:
    std::string m_adbPath;
    std::vector<std::string> m_params;
    DGLBaseQTProcess* m_process;
    bool m_Deleted;
};

class DGLAdbOutputFilter {
   public:
    virtual bool filter(const std::vector<std::string>& input,
                        std::vector<std::string>& output) = 0;
    virtual ~DGLAdbOutputFilter() {}
};

class DGLAdbCookieFactoryBase {
   public:
    virtual DGLAdbCookie* CreateCookie(
            const std::vector<std::string>& params, DGLAdbHandler* handler,
            std::shared_ptr<DGLAdbOutputFilter> filter) = 0;
};

class DGLAdbCookieFactory : public DGLAdbCookieFactoryBase {
   public:
    DGLAdbCookieFactory(const std::string adbPath);
    const std::string& getAdbPath();

   private:
    virtual DGLAdbCookie* CreateCookie(
            const std::vector<std::string>& params, DGLAdbHandler* handler,
            std::shared_ptr<DGLAdbOutputFilter> filter) override;

    std::string m_adbPath;
};

class DGLAdbInterface {
   public:
    static DGLAdbInterface* get();

    void setAdbCookieFactory(std::shared_ptr<DGLAdbCookieFactoryBase>);
    const std::string getAdbPath() const;

    DGLAdbCookie* killServer(DGLAdbHandler* handler);
    DGLAdbCookie* connect(const std::string& address, DGLAdbHandler* handler);
    DGLAdbCookie* getDevices(DGLAdbHandler* handler);

    DGLAdbCookie* invokeOnDevice(const std::string& serial,
                                 const std::vector<std::string>& params,
                                 DGLAdbHandler* handler,
                                 std::shared_ptr<DGLAdbOutputFilter> filter =
                                         std::shared_ptr<DGLAdbOutputFilter>());

   private:
    DGLAdbCookie* invokeAdb(const std::vector<std::string>& params,
                            DGLAdbHandler* handler,
                            std::shared_ptr<DGLAdbOutputFilter> filter =
                                    std::shared_ptr<DGLAdbOutputFilter>());

    std::shared_ptr<DGLAdbCookieFactoryBase> m_factory;
    static std::shared_ptr<DGLAdbInterface> s_self;
};

#endif
