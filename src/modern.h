#include <stdint.h>

#include <memory>

namespace Modern {
class Container {
   public:
    Container(size_t size);
    ~Container();
    friend void swap(Container& first, Container& second);
    Container(const Container& other);
    Container(Container&& other) noexcept;
    Container& operator=(Container other);
    int32_t& operator[](int32_t i);

   private:
    std::unique_ptr<int32_t[]> data;
    const size_t size;
};

void move_semantics();
void modern();
}  // namespace Modern
