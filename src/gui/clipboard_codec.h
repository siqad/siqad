#pragma once
#include <QByteArray>
#include <QJsonArray>
#include <QString>

namespace gui::clipboard_codec {
inline constexpr auto mimeType = "application/x-siqad-sidb-selection";
inline constexpr int maxBytes = 1024 * 1024;
inline constexpr int maxItems = 10000;
inline constexpr int maxDepth = 64;

// Pure QtCore validation. Output changes only after successful validation.
// basisSize == 0 validates the portable format without a target lattice.
bool decode(const QByteArray &payload, int basisSize, QJsonArray &items, QString *error = nullptr);
QByteArray encode(const QJsonArray &items);
}
