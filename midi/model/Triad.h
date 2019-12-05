
#include <assert.h>
#include <memory>
#include <vector>

class ScaleRelativeNote;
class Triad;
class Scale;

using ScaleRelativeNotePtr = std::shared_ptr<ScaleRelativeNote>;
using TriadPtr = std::shared_ptr<Triad>;
using ScalePtr = std::shared_ptr<Scale>;

class Triad
{
public:
    enum class Inversion { Root, First, Second};
    static TriadPtr make(ScalePtr scale, const ScaleRelativeNote& root, Inversion);

    void assertValid() const;

    ScaleRelativeNotePtr get(int index) const
    {
        assert(index >= 0 && index <= 2);
        return notes[index];
    }

    std::vector<float> toCv(ScalePtr scale) const;
private:
    Triad();
    std::vector<ScaleRelativeNotePtr> notes;

};