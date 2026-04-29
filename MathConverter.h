#ifndef MATHCONVERTER_H
#define MATHCONVERTER_H

#include <QString>

class MathConverter
{
public:
    // Convert LaTeX-style math input to Unicode
    // Returns the converted string and how many characters to delete
    static QString convert(const QString &input, int &charsToDelete);
    
    // Evaluate inline math expression (e.g. "5+3=" → "8")
    static QString evaluate(const QString &input, int &charsToDelete);

private:
    // Helper to check if character at position is preceded by word boundary
    static bool hasWordBoundaryBefore(const QString &text, int pos);
};

#endif // MATHCONVERTER_H
