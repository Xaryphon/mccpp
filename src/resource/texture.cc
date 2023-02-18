#include "texture.hh"

#include <spng.h>

#include "../logger.hh"
#include "../utility/scope_guard.hh"

namespace mccpp::resource {

texture_object::texture_object(manager &mgr, const identifier &id, load_flags flags) {
    (void)flags;

    {
        auto png = mgr.read_file("{}/textures/{}", id.name_space(), id.name());
        spng_ctx *ctx = spng_ctx_new(0);
        if (!ctx) {
            MCCPP_W("{}: spng_ctx_new failed.", id.full());
            goto error_image;
        }
        MCCPP_SCOPE_EXIT { spng_ctx_free(ctx); };

        int error = spng_set_png_buffer(ctx, png.data(), png.size());
        if (error) {
            MCCPP_W("{}: spng_set_png_buffer failed: {}", id.full(), spng_strerror(error));
            goto error_image;
        }

        spng_ihdr ihdr;
        error = spng_get_ihdr(ctx, &ihdr);
        if (error) {
            MCCPP_W("{}: spng_get_ihdr failed: {}", id.full(), spng_strerror(error));
            goto error_image;
        }

        size_t texture_size;
        error = spng_decoded_image_size(ctx, SPNG_FMT_RGB8, &texture_size);
        if (error) {
            MCCPP_W("{}: spng_decoded_image_size failed: {}", id.full(), spng_strerror(error));
            goto error_image;
        }

        m_pixels = runtime_array<std::byte>(texture_size);
        error = spng_decode_image(ctx, m_pixels.data(), m_pixels.size(), SPNG_FMT_RGB8, 0);
        if (error) {
            MCCPP_W("{}: spng_decode_image failed: {}", id.full(), spng_strerror(error));
            goto error_image;
        }

        m_width = ihdr.width;
        m_height = ihdr.height;
    }

    return;

error_image:
    m_pixels = runtime_array<std::byte>(16);
    std::byte *ptr = m_pixels.data();

    *ptr++ = std::byte(0xff);
    *ptr++ = std::byte(0x00);
    *ptr++ = std::byte(0xff);
    *ptr++ = std::byte(0xff);

    *ptr++ = std::byte(0x00);
    *ptr++ = std::byte(0x00);
    *ptr++ = std::byte(0x00);
    *ptr++ = std::byte(0xff);

    *ptr++ = std::byte(0x00);
    *ptr++ = std::byte(0x00);
    *ptr++ = std::byte(0x00);
    *ptr++ = std::byte(0xff);

    *ptr++ = std::byte(0xff);
    *ptr++ = std::byte(0x00);
    *ptr++ = std::byte(0xff);
    *ptr++ = std::byte(0xff);
}

}
