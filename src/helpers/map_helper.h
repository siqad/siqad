#include <QMap>
#include <QSet>

template<typename K, typename V>
QList<K> getUniqueKeys(const QMap<K, V>& map) {
  QSet<K> keys;
  for (auto it = map.cbegin(); it != map.cend(); ++it)
  {
    keys.insert(it.key());
  }
  return keys.values(); // Convert QSet to QList
}

template<typename K, typename V>
QSet<K> getUniqueKeySet(const QMap<K, V>& map) {
  QSet<K> keys;
  for (auto it = map.cbegin(); it != map.cend(); ++it)
  {
    keys.insert(it.key());
  }
  return keys;
}
