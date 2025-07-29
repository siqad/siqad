#include "../libs/miniz/miniz.h"
#include <QString>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

bool add_directory_to_zip(mz_zip_archive &zip, const std::string &folder_path, const std::string &base_path = "") {
  try {
    for (const auto &entry : fs::recursive_directory_iterator(folder_path)) {
      if (fs::is_directory(entry.path())) {
        continue; // Skip directories themselves (just include files)
      }
      std::string file_path = entry.path().string();
      std::string zip_name = base_path.empty() ? file_path.substr(folder_path.length() + 1)
                                               : base_path + "/" + file_path.substr(folder_path.length() + 1);

      // Add the file to the zip archive
      if (!mz_zip_writer_add_file(&zip, zip_name.c_str(), file_path.c_str(), "", 0, MZ_BEST_COMPRESSION)) {
        std::cerr << "Error adding file: " << file_path << std::endl;
        return false;
      }
    }
  }
  catch (const fs::filesystem_error &e) {
    std::cerr << "Filesystem error: " << e.what() << std::endl;
    return false;
  }
  return true;
}

bool zip_directory(const std::string &source_dir, const std::string &archive_filename)
{
  mz_zip_archive zip_archive;
  memset(&zip_archive, 0, sizeof(zip_archive));

  // Initialize a new zip archive
  if (!mz_zip_writer_init_file(&zip_archive, archive_filename.c_str(), 0)) {
    std::cerr << "Could not initialize zip archive" << std::endl;
    return false;
  }

  // Add the directory to the zip
  if (!add_directory_to_zip(zip_archive, source_dir)) {
    mz_zip_writer_end(&zip_archive);
    return false;
  }

  // Finalize the archive
  if (!mz_zip_writer_finalize_archive(&zip_archive)) {
    std::cerr << "Could not finalize zip archive" << std::endl;
    mz_zip_writer_end(&zip_archive);
    return false;
  }

  mz_zip_writer_end(&zip_archive);
  return true;
}

// Helper function to extract all contents of the zip archive
bool extract_all_from_zip(const QString &zipFilePath, const QString &destinationDir) {
  mz_zip_archive zip_archive;
  memset(&zip_archive, 0, sizeof(zip_archive));

  if (!mz_zip_reader_init_file(&zip_archive, zipFilePath.toStdString().c_str(), 0)) {
    qDebug() << "Could not initialize zip reader";
    return false;
  }

  int num_files = mz_zip_reader_get_num_files(&zip_archive);
  for (int i = 0; i < num_files; i++) {
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
      qDebug() << "Could not read file stat";
      mz_zip_reader_end(&zip_archive);
      return false;
    }

    QString extracted_path = destinationDir + "/" + QString::fromUtf8(file_stat.m_filename);

    if (file_stat.m_is_directory) {
      qDebug() << "Make directory: " << extracted_path;
      QDir().mkpath(extracted_path);  // Create directory path if necessary
    } else {
      // Ensure the file's directory exists
      QDir dir = QFileInfo(extracted_path).dir();
      if (!dir.exists()) {
        dir.mkpath("."); // Create all necessary parent directories
      }

      qDebug() << "Extracting file: " << extracted_path;
      if (!mz_zip_reader_extract_to_file(&zip_archive, i, extracted_path.toStdString().c_str(), 0)) {
        qDebug() << "Could not extract file: " << extracted_path;
        mz_zip_reader_end(&zip_archive);
        return false;
      }
    }
  }

  mz_zip_reader_end(&zip_archive);
  return true;
}
