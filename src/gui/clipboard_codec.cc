#include "clipboard_codec.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <cmath>
#include <limits>

namespace gui::clipboard_codec {
namespace {
bool integer(const QJsonValue &value) {
  const double n = value.toDouble(std::numeric_limits<double>::quiet_NaN());
  return value.isDouble() && std::isfinite(n) && std::trunc(n) == n
      && n >= std::numeric_limits<int>::min() && n <= std::numeric_limits<int>::max();
}
bool point(const QJsonValue &value) {
  if (!value.isObject()) return false;
  const auto p = value.toObject();
  return p["x"].isDouble() && p["y"].isDouble()
      && std::isfinite(p["x"].toDouble()) && std::isfinite(p["y"].toDouble());
}
bool validate(const QJsonArray &items, int basisSize, int depth, int &count, QSet<QString> &sites) {
  if (depth > maxDepth || items.isEmpty()) return false;
  for (const auto &value : items) {
    if (++count > maxItems || !value.isObject()) return false;
    const auto item = value.toObject();
    if (!integer(item["layer"]) || item["layer"].toInt() < 0 || !point(item["pos"])) return false;
    if (item["type"] == "db") {
      const auto coord = item["lat"].toObject();
      if (!integer(coord["n"]) || !integer(coord["m"]) || !integer(coord["l"])) return false;
      const int l = coord["l"].toInt();
      if (l < 0 || (basisSize > 0 && l >= basisSize)) return false;
      const auto site = QString("%1,%2,%3").arg(coord["n"].toInt()).arg(coord["m"].toInt()).arg(l);
      if (sites.contains(site)) return false;
      sites.insert(site);
    } else if (item["type"] == "aggregate") {
      if (!item["children"].isArray()
          || !validate(item["children"].toArray(), basisSize, depth + 1, count, sites)) return false;
    } else return false;
  }
  return true;
}
}

bool decode(const QByteArray &payload, int basisSize, QJsonArray &items, QString *error) {
  const auto reject = [error](const char *reason) { if (error) *error = reason; return false; };
  if (payload.isEmpty() || payload.size() > maxBytes || basisSize < 0) return reject("Invalid payload size or lattice");
  QJsonParseError parseError;
  const auto doc = QJsonDocument::fromJson(payload, &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) return reject("Invalid JSON document");
  const auto root = doc.object();
  if (root["format"] != "siqad-sidb-selection" || !integer(root["version"])
      || root["version"].toInt() != 1 || !root["items"].isArray()) return reject("Unsupported clipboard format");
  const auto decoded = root["items"].toArray();
  int count = 0;
  QSet<QString> sites;
  if (!validate(decoded, basisSize, 1, count, sites)) return reject("Invalid or excessive item tree");
  items = decoded;
  if (error) error->clear();
  return true;
}

QByteArray encode(const QJsonArray &items) {
  int count = 0;
  QSet<QString> sites;
  if (!validate(items, 0, 1, count, sites)) return {};
  const auto bytes = QJsonDocument(QJsonObject{{"format", "siqad-sidb-selection"},
      {"version", 1}, {"items", items}}).toJson(QJsonDocument::Compact);
  return bytes.size() <= maxBytes ? bytes : QByteArray{};
}
}
