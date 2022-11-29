#include "texture.hh"

#include <cassert>
#include <spng.h>

#include "manager.hh"

namespace mccpp::resource {

texture::texture(const char *path)
{
    load(path);
}

texture::~texture()
{

}

void texture::load(const char *path)
{
    // FIXME: Handle spng errors
    auto png = manager::read_file(path);
    spng_ctx *ctx = spng_ctx_new(0);
    assert(ctx);
    int error = spng_set_png_buffer(ctx, png.data(), png.size());
    assert(error == 0);
    spng_ihdr ihdr;
    error = spng_get_ihdr(ctx, &ihdr);
    assert(error == 0);
    size_t texture_size;
    error = spng_decoded_image_size(ctx, SPNG_FMT_RGB8, &texture_size);
    assert(error == 0);
    auto texture_data = std::make_unique<char[]>(texture_size);
    error = spng_decode_image(ctx, texture_data.get(), texture_size, SPNG_FMT_RGB8, 0);
    assert(error == 0);
    spng_ctx_free(ctx);

    m_width = ihdr.width;
    m_height = ihdr.height;
    m_length = texture_size;
    m_data = std::move(texture_data);
}

uint32_t texture::width()
{
    return m_width;
}

uint32_t texture::height()
{
    return m_height;
}

void *texture::data()
{
    return m_data.get();
}

size_t texture::length()
{
    return m_length;
}

}
