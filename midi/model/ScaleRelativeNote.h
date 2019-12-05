#pragma once
#include <memory>

class ScaleRelativeNote;
class Scale;
using ScaleRelativeNotePtr = std::shared_ptr<ScaleRelativeNote>;

class ScaleRelativeNote
{
public:
    friend Scale;
   

    const bool valid; 
    const int degree;
    const int octave;

  //  ScaleRelativeNotePtr clone() const;

    /**
     * factory method for tests.
     * Try to avoid using this in real code - Scale has safer APIs.
     */ 
    static ScaleRelativeNote _testMakeFromDegreeAndOctave(int degree, int octave)
    {
        return ScaleRelativeNote(degree, octave);
    }
private:
  // make ctors private so only scale can make them
  ScaleRelativeNote(int degree, int octave);

  // This ctor makes an invalid one
  ScaleRelativeNote() : valid(false), degree(-1), octave(0)
  {

  }

};