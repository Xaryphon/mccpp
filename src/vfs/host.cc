// FIXME: Currently uses POSIX dirent. Figure out a good cross-platform way to do this.
#include "host.hh"

#include <cassert>
#include <cstring>
#include <dirent.h>
#include <fstream>

#include "logger.hh"
#include "utility/scope_guard.hh"

namespace mccpp::vfs {

static DIR *opendir_or_throw(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        throw std::system_error(errno, std::generic_category(), path);
    }
    return dir;
}

static dirent *readdir_or_throw(DIR *dir) {
    errno = 0;
    dirent *ent = readdir(dir);
    if (ent == nullptr && errno != 0) {
        throw std::system_error(errno, std::generic_category(), "readdir");
    }
    return ent;
}

static dirent *readdir_helper(DIR *dir) {
    dirent *ent;
    do {
        ent = readdir_or_throw(dir);
        if (ent == nullptr)
            return nullptr;
    } while (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0);
    return ent;
}

class host_storage_iterator final : public storage_iterator {
public:
    static std::unique_ptr<host_storage_iterator> create(std::string &&root) {
        return std::make_unique<host_storage_iterator>(std::move(root));
    }

    host_storage_iterator(std::string &&root)
    : m_path(std::move(root))
    , m_iterators(1, opendir_or_throw(m_path.c_str()))
    , m_dirent(nullptr)
    {}

    ~host_storage_iterator() {
        for (DIR *dir : m_iterators) {
            closedir(dir);
        }
    }

    bool is_directory() override {
        return m_dirent->d_type == DT_DIR;
    }

    std::string_view name() override {
        return m_dirent->d_name;
    }

    bool next() override {
        assert(m_iterators.size() > 0);

        m_dirent = readdir_helper(m_iterators.back());
        if (m_dirent == nullptr) {
            closedir(m_iterators.back());
            m_iterators.pop_back();
            if (!m_iterators.empty()) {
                m_path.erase(m_path.rfind('/'));
            }
            return false;
        }

        if (is_directory()) {
            m_path += '/';
            m_path += m_dirent->d_name;
            m_iterators.emplace_back(opendir_or_throw(m_path.c_str()));
        }

        return true;
    }

private:
    std::string m_path;
    std::vector<DIR *> m_iterators;
    dirent *m_dirent;
};

std::unique_ptr<storage_iterator> host_storage::create_iterator() const {
    return host_storage_iterator::create(std::string(m_root));
}

runtime_array<std::byte> host_storage::read_file(std::string_view path) const {
    std::filebuf *filebuf;
    std::ifstream stream;
    std::string full_path = m_root;
    full_path += path;
    stream.open(full_path, std::ios_base::in | std::ios_base::binary);
    if (stream.fail()) {
        MCCPP_E("Failed to open file {}", full_path);
        return runtime_array<std::byte>();
    }
    filebuf = stream.rdbuf();

    auto size = filebuf->pubseekoff(0, std::ios::end, std::ios::in);
    if (size < 0) {
        MCCPP_E("Failed to seek to end of {}", full_path);
        return runtime_array<std::byte>();
    }
    filebuf->pubseekpos(0, std::ios::in);

    runtime_array<std::byte> buffer { (size_t)size };
    filebuf->sgetn(reinterpret_cast<char*>(buffer.data()), buffer.size());

    stream.close();
    return buffer;
}

}
