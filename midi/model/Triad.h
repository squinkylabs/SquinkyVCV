
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
    static TriadPtr make(ScalePtr scale, const ScaleRelativeNote& root, const Triad& previousTriad);

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

    static float ratePair(ScalePtr scale, const Triad& first, const Triad& second);
    static bool isParallel(const std::vector<float>& first, const std::vector<float>& second);
    static float sumDistance(const std::vector<float>& first, const std::vector<float>& second);
};