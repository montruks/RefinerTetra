#ifndef REFINERTETRA_HPP
#define REFINERTETRA_HPP

#include "GenericMesh.hpp"
#include "Output.hpp"

#include <set>
#include <iterator>

namespace GeDiM
{
    class RefinerTetra
    {
        protected:

            GenericMesh* meshPointer;
            vector<unsigned int> idCellToRefine;
            set<unsigned int > idCellsToUpdate;

            const Output::ExitCodes FindMaxEdge(GenericCell& cell, double toll);
            const Output::ExitCodes CutTetra(GenericCell& cell);
            const Output::ExitCodes RecoverConformity();

        public:

            RefinerTetra();
            virtual ~RefinerTetra();

            void SetMesh(GenericMesh& mesh) {meshPointer = &mesh;}
            const Output::ExitCodes InitializeIdCells(const unsigned int& numberOfCells) { idCellToRefine.reserve(numberOfCells); return Output::Success;}
            const Output::ExitCodes	AddIdCell(const unsigned int& idCell);
            const Output::ExitCodes RefineMesh();

    };
}

#endif // REFINERTETRA_H
