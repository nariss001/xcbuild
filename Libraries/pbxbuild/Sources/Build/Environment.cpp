/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <pbxbuild/Build/Environment.h>
#include <xcsdk/Configuration.h>
#include <xcsdk/Environment.h>
#include <pbxsetting/DefaultSettings.h>
#include <pbxsetting/Environment.h>
#include <libutil/Filesystem.h>

namespace Build = pbxbuild::Build;
using libutil::Filesystem;

Build::Environment::
Environment(pbxspec::Manager::shared_ptr const &specManager, std::shared_ptr<xcsdk::SDK::Manager> const &sdkManager, pbxsetting::Environment const &baseEnvironment) :
    _specManager(specManager),
    _sdkManager(sdkManager),
    _baseEnvironment(baseEnvironment)
{
}

ext::optional<Build::Environment> Build::Environment::
Default(process::Context const *processContext, Filesystem const *filesystem)
{
    ext::optional<std::string> developerRoot = xcsdk::Environment::DeveloperRoot(processContext, filesystem);
    if (!developerRoot) {
        fprintf(stderr, "error: couldn't find developer dir\n");
        return ext::nullopt;
    }

    auto configuration = xcsdk::Configuration::Load(filesystem, xcsdk::Configuration::DefaultPaths(processContext));
    auto sdkManager = xcsdk::SDK::Manager::Open(filesystem, *developerRoot, configuration);
    if (sdkManager == nullptr) {
        fprintf(stderr, "error: couldn't create SDK manager\n");
        return ext::nullopt;
    }

    /*
     * Collect specification domains to register.
     */
    std::vector<std::pair<std::string, std::string>> domains;

    /* Global specifications. */
    std::vector<std::pair<std::string, std::string>> defaultDomains = pbxspec::Manager::DefaultDomains(*developerRoot);
    domains.insert(domains.end(), defaultDomains.begin(), defaultDomains.end());

    /* Platform-specific specifications. */
    std::unordered_map<std::string, std::string> platforms;
    for (xcsdk::SDK::Platform::shared_ptr const &platform : sdkManager->platforms()) {
        platforms.insert({ platform->name(), platform->path() });
    }
    std::vector<std::pair<std::string, std::string>> platformDomains = pbxspec::Manager::PlatformDomains(platforms);
    domains.insert(domains.end(), platformDomains.begin(), platformDomains.end());

    /* Platform-dependent specifications. */
    std::vector<std::pair<std::string, std::string>> platformDependentDomains = pbxspec::Manager::PlatformDependentDomains(*developerRoot);
    domains.insert(domains.end(), platformDependentDomains.begin(), platformDependentDomains.end());

    /* Global build rules. */
    std::vector<std::string> buildRules;
    for (std::string const &path : pbxspec::Manager::DeveloperBuildRules(*developerRoot)) {
        if (filesystem->isReadable(path)) {
            buildRules.push_back(path);
        }
    }

    /*
     * Load specification domains.
     */
    auto specManager = pbxspec::Manager::Create(filesystem, buildRules, domains);
    if (specManager == nullptr) {
        fprintf(stderr, "error: couldn't create spec manager\n");
        return ext::nullopt;
    }

    pbxspec::PBX::BuildSystem::shared_ptr buildSystem = specManager->buildSystem("com.apple.build-system.core", { "default" });
    if (buildSystem == nullptr) {
        fprintf(stderr, "error: couldn't create build system\n");
        return ext::nullopt;
    }

    pbxsetting::Environment baseEnvironment;
    baseEnvironment.insertBack(buildSystem->defaultSettings(), true);
    baseEnvironment.insertBack(sdkManager->computedSettings(), false);
    std::vector<pbxsetting::Level> defaultLevels = pbxsetting::DefaultSettings::Levels(processContext);
    for (pbxsetting::Level const &level : defaultLevels) {
        baseEnvironment.insertBack(level, false);
    }

    return Build::Environment(specManager, sdkManager, baseEnvironment);
}
