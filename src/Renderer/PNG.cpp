#include "PNG.h"

PNG::PNG(std::shared_ptr<ResourceBundle::Block> block)
{
	png_byte header[8];
    block->Stream(header, 8);
	bool isPNG = !png_sig_cmp(header, 0, 8);
	if (!isPNG) {
        throw Resource::FailedLoadingException("File is not PNG.");
	}

	// Initialize libpng
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, (png_error_ptr)&PNG::pngErrorFunction, (png_error_ptr)&PNG::pngErrorFunction);
	if (!png_ptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        throw Resource::FailedLoadingException("Failed to initialze png_struct.");
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        throw Resource::FailedLoadingException("Failed to initialze png_info.");
	}
	png_infop info_end_ptr = png_create_info_struct(png_ptr);
	if (!info_end_ptr) {
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        throw Resource::FailedLoadingException("Failed to initialze second png_info.");
	}

	png_set_read_fn(png_ptr, &block, &PNG::pngReadFunction);

	// We already read the first 8 bytes of the header
	png_set_sig_bytes(png_ptr, 8);
	// Read all the info up to the image data
	png_read_info(png_ptr, info_ptr);

	// Get info
	int bit_depth, color_type;
	unsigned int width, height;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
	if (bit_depth != 8) {
        throw Resource::FailedLoadingException("Unsupported bit depth. Must be 8");

	}
	switch (color_type) {
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGBA:
			Format = Image::ImageFormat::RGBA;
			break;
		default:
            throw Resource::FailedLoadingException("Unsupported color format.");
	}

	// Convert RGB to RGBA, since DirectX rather treat them all the same way
	if (color_type == PNG_COLOR_TYPE_RGB) {
		LOG_DEBUG("Converting RGB to RGBA for \"%s\"", block->Path().c_str());
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
		png_read_update_info(png_ptr, info_ptr);
	}
	
	std::size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	m_DataSize = height * row_bytes;
	this->Data = new unsigned char[m_DataSize];
	png_bytep* row_pointers = new png_bytep[height];

	// Point each row to the continuous data array
	for (unsigned int i = 0; i < height; ++i) {
		// Invert Y for OpenGL
		row_pointers[height - 1 - i] = this->Data + i * row_bytes;
	}

	// Read in the data
	png_read_image(png_ptr, row_pointers);
	delete[] row_pointers;

	this->Width = width;
	this->Height = height;
	auto bytePerPixel = bit_depth / 8 * 4;	
	this->ByteSize = width * height * bytePerPixel;
	png_destroy_read_struct(&png_ptr, &info_ptr, &info_end_ptr);
}

std::size_t PNG::Size()
{
	return m_DataSize;
}

PNG::~PNG()
{
	if (this->Data != nullptr) {
		delete[] this->Data;
		this->Data = nullptr;
	}
}

void PNG::pngErrorFunction(png_structp png_ptr, png_const_charp error_msg)
{
	LOG_WARNING("libpng error: %s", error_msg);
}

void PNG::pngWarningFunction(png_structp png_ptr, png_const_charp warning_msg)
{
	LOG_WARNING("libpng warning: %s", warning_msg);
}

void PNG::pngReadFunction(png_structp png_ptr, png_bytep data, png_size_t length)
{
	auto& block = *reinterpret_cast<std::shared_ptr<ResourceBundle::Block>*>(png_get_io_ptr(png_ptr));
	block->Stream(data, length);
}
