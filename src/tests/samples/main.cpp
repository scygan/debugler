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

#include "sample.h"
#include "platform.h"

#pragma warning(push)
#pragma warning( \
        disable  \
        : 4512)    //'boost::program_options::options_description' : \
                           //assignment operator could not be generated
#include <boost/program_options/options_description.hpp>
#pragma warning(pop)
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

int main(int argc, char** argv) {

    try {
        po::options_description desc("Allowed options");
        desc.add_options()("help,h", "produce help message");

        po::options_description mandatory("Mandatory options");
        mandatory.add_options()(
                "sample", po::value<std::vector<std::string> >()->composing(),
                "Sample to execute (default is \"simple\").");

        po::positional_options_description p;
        p.add("sample", -1);

        po::options_description all;
        all.add(desc).add(mandatory);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
                          .options(all)
                          .positional(p)
                          .run(),
                  vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::ostringstream out;
            out << desc << "\n" << mandatory << "\n";
            throw std::runtime_error(out.str());
        }

        std::string sampleName = "simple";

        if (vm.count("sample")) {
            sampleName = vm["sample"].as<std::vector<std::string> >()[0];
        }

        Platform platform;

        std::shared_ptr<PlatWindowCtx> window = platform.createWindow();

        window->makeCurrent();

        std::shared_ptr<Sample> sample = Sample::getSample(sampleName);

        sample->setWindow(window.get());

        sample->startup();
        window->swapBuffers();

        while (!window->pendingClose()) {
            sample->render();
            window->swapBuffers();
            platform.pollEvents();
        }

        sample->shutdown();
    }
    catch (const std::runtime_error& error) {

        fprintf(stderr, "Error: %s\n", error.what());

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
