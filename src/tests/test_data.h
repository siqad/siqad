#pragma once
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace test_data {
inline QJsonObject db(int n = 0, int m = 0, int l = 0, int layer = 2) {
  return {{"type", "db"}, {"layer", layer},
      {"pos", QJsonObject{{"x", n * 384.0}, {"y", m * 768.0 + l * 225.0}}},
      {"lat", QJsonObject{{"n", n}, {"m", m}, {"l", l}}}};
}
inline QJsonObject aggregate(const QJsonArray &children) {
  return {{"type", "aggregate"}, {"layer", 2},
      {"pos", QJsonObject{{"x", 0}, {"y", 0}}}, {"children", children}};
}
inline QByteArray payload(const QJsonArray &items, QJsonValue version = 1) {
  return QJsonDocument(QJsonObject{{"format", "siqad-sidb-selection"},
      {"version", version}, {"items", items}}).toJson(QJsonDocument::Compact);
}
}
