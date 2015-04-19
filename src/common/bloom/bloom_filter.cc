#include "common/bloom_filter.h"
#include <exception>

extern "C" {
    #include <sys/mman.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
}

namespace hypocampd {

    uint8_t* NewCreate::allocate(size_t num_bytes) {

        uint8_t* buffer = nullptr;
        try 
        {
            buffer = new uint8_t[num_bytes]();
        }
        catch (std::bad_alloc& e)
        {
            ERROR("No more memory left!");
            return nullptr; 
        }
        return buffer;
    }

    void NewCreate::deallocate(uint8_t* buffer, size_t size) {
        delete[] buffer;
    }


    uint8_t* MmapCreate::allocate(size_t num_bytes) {
        static int fd;
        void* buffer = nullptr;

        fd = ::open("/dev/null", O_RDWR);
        if (fd == -1) {
            ERROR("Failed to open file");
            return nullptr;
        }

        buffer = ::mmap(NULL, num_bytes, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

        if (buffer == MAP_FAILED) {
            ERROR("Failed to allocate via mmap");
            return nullptr;
        }

        return static_cast<uint8_t*>(buffer);
    }

    
    void MmapCreate::deallocate(uint8_t* buffer, size_t size) {
        if (::munmap((void*)buffer, size)) {
            ERROR("Failed to close mapped memory region");
        }
    }

}; // END namespace hypocampd
