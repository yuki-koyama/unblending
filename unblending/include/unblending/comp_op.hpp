#ifndef COMP_OP_HPP
#define COMP_OP_HPP

#include <unblending/common.hpp>

namespace unblending
{
    // CompOp is a class for describing Porter-Duff's composition operations
    class CompOp
    {
    public:
        CompOp(int X = 0, int Y = 0, int Z = 0) : X(X), Y(Y), Z(Z) {}
        
        int X;
        int Y;
        int Z;
        
        bool is_source_over() const { return X == 1 && Y == 1 && Z == 1; }
        bool is_plus()        const { return X == 2 && Y == 1 && Z == 1; }
        
        static CompOp SourceOver()
        {
            return CompOp(1, 1, 1);
        }
        
        static CompOp Plus()
        {
            return CompOp(2, 1, 1);
        }
    };
}

#endif // COMP_OP_HPP
