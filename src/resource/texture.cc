#include "texture.hh"

#include <spng.h>

#include "../logger.hh"
#include "../utility/scope_guard.hh"

namespace mccpp::resource {

bool texture_object::do_load(bool force_reload) {
    (void)force_reload;

    auto png = read_file(resource_path());
    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx) {
        MCCPP_W("{}: spng_ctx_new failed.", resource_path());
        return false;
    }
    MCCPP_SCOPE_EXIT { spng_ctx_free(ctx); };

    int error = spng_set_png_buffer(ctx, png.data(), png.size());
    if (error) {
        MCCPP_W("{}: spng_set_png_buffer failed: {}", resource_path(), spng_strerror(error));
        return false;
    }

    spng_ihdr ihdr;
    error = spng_get_ihdr(ctx, &ihdr);
    if (error) {
        MCCPP_W("{}: spng_get_ihdr failed: {}", resource_path(), spng_strerror(error));
        return false;
    }

    size_t texture_size;
    error = spng_decoded_image_size(ctx, SPNG_FMT_RGB8, &texture_size);
    if (error) {
        MCCPP_W("{}: spng_decoded_image_size failed: {}", resource_path(), spng_strerror(error));
        return false;
    }

    m_pixels = runtime_array<std::byte>(texture_size);
    error = spng_decode_image(ctx, m_pixels.data(), m_pixels.size(), SPNG_FMT_RGB8, 0);
    if (error) {
        MCCPP_W("{}: spng_decode_image failed: {}", resource_path(), spng_strerror(error));
        return false;
    }

    m_width = ihdr.width;
    m_height = ihdr.height;

    return true;
}

}
