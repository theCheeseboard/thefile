#include "isoftwareupdatefile.h"

#include <plist/plist++.h>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

struct ISoftwareUpdateFilePrivate {
        bool valid = false;
        QString productVersion;
        QString productBuildVersion;
        QStringList supportedProductTypes;
};

ISoftwareUpdateFile::ISoftwareUpdateFile(QString path, QObject* parent) :
    QObject{parent} {
    d = new ISoftwareUpdateFilePrivate();

    auto file = QuaZipFile(path, "BuildManifest.plist");
    if (!file.open(QuaZipFile::ReadOnly)) return;
    auto plistData = file.readAll();
    file.close();

    plist_t plistHandle;
    if (plist_from_memory(plistData.data(), plistData.length(), &plistHandle) != PLIST_ERR_SUCCESS) return;

    auto node = PList::Dictionary(plistHandle);
    auto productVersionIterator = node.Find("ProductVersion");
    auto productBuildVersionIterator = node.Find("ProductBuildVersion");
    auto supportedProductTypesIterator = node.Find("SupportedProductTypes");
    if (productVersionIterator == node.End() || productBuildVersionIterator == node.End() || supportedProductTypesIterator == node.End()) {
        return;
    }

    auto productVersionString = static_cast<PList::String*>(productVersionIterator->second);
    d->productVersion = QString::fromStdString(productVersionString->GetValue());
    auto productBuildVersionString = static_cast<PList::String*>(productBuildVersionIterator->second);
    d->productBuildVersion = QString::fromStdString(productBuildVersionString->GetValue());

    auto supportedProductTypesArray = static_cast<PList::Array*>(supportedProductTypesIterator->second);
    for (auto i = 0; i < supportedProductTypesArray->GetSize(); i++) {
        auto node = supportedProductTypesArray->operator[](i);
        if (node->GetType() != PLIST_STRING) {
            return;
        }

        auto typeString = static_cast<PList::String*>(node);
        d->supportedProductTypes.append(QString::fromStdString(typeString->GetValue()));
    }

    d->valid = true;
}

ISoftwareUpdateFile::~ISoftwareUpdateFile() {
    delete d;
}

bool ISoftwareUpdateFile::isValid() {
    return d->valid;
}

QString ISoftwareUpdateFile::productVersion() {
    return d->productVersion;
}

QString ISoftwareUpdateFile::productBuildVersion() {
    return d->productBuildVersion;
}

QStringList ISoftwareUpdateFile::supportedProductTypes() {
    return d->supportedProductTypes;
}
