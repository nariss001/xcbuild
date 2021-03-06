/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __xcassets_Asset_BrandAssets_h
#define __xcassets_Asset_BrandAssets_h

#include <xcassets/Asset/Asset.h>
#include <plist/Dictionary.h>

#include <memory>
#include <string>
#include <vector>
#include <ext/optional>

namespace xcassets {
namespace Asset {

class ImageSet;

class BrandAssets : public Asset {
private:
    // TODO: image set and image stack
    std::vector<std::shared_ptr<Asset>> _children;

private:
    // TODO: assets

private:
    friend class Asset;
    using Asset::Asset;

public:
    // TODO: image set and image stack
    std::vector<std::shared_ptr<Asset>> const &children() const
    { return _children; }

public:
    // TODO: assets

public:
    static AssetType Type()
    { return AssetType::BrandAssets; }
    virtual AssetType type()
    { return AssetType::BrandAssets; }

public:
    static ext::optional<std::string> Extension()
    { return std::string("brandassets"); }

protected:
    virtual bool load(libutil::Filesystem const *filesystem);
    virtual bool parse(plist::Dictionary const *dict, std::unordered_set<std::string> *seen, bool check);
};

}
}

#endif // !__xcassets_Asset_BrandAssets_h

