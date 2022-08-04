#include "RefinerTetra.hpp"

#define DEBUG

#ifdef DEBUG
#include <iostream>
#endif // DEBUG

using namespace MainApplication;

namespace GeDiM
{
    RefinerTetra::RefinerTetra()
    {

    }
    // ***************************************************************************
    RefinerTetra::~RefinerTetra()
    {

    }
    // ***************************************************************************
    const Output::ExitCodes RefinerTetra::FindMaxEdge(GenericCell& cell, double toll)
    {
        ///dichiarazine variabili

        unsigned int inactiveFaces = 0;
        unsigned int inactiveEdges = 0;
        unsigned int iCount = 0; //conta facce inattivate dal lato
        unsigned int ii = 0; //indice lato con più facce inattive
        double maxLenght = 0; //massima lunghezza
        unsigned int il = 0; //indice lato più lungo
        bool active = true;

        ///calcolo valori parametri

        for(unsigned int i = 0; i<4; i++)
        {
            unsigned int idFace = cell.Face(i)->Id();
            GenericFace& face = *meshPointer->Face(idFace);
            if(!face.IsActive())
                inactiveFaces++;
        }
        for(unsigned int i = 0; i<6; i++)
        {
            unsigned int idEdge = cell.Edge(i)->Id();
            GenericEdge& edge = *meshPointer->Edge(idEdge);

            // Cerco il lato con il maggior numero di facce aventi un taglio lungo quel lato
            // Con taglio lungo quel lato si intende un taglio che va dal vertice opposto al punto medio del lato considerato

            unsigned int numFac = 0;

            if(!edge.IsActive())
            {
                inactiveEdges++;

                for (unsigned int j = 0; j < 4; j++)
                {
                    unsigned int idFace = cell.Face(j)->Id();
                    GenericFace& face = *meshPointer->Face(idFace);
                    if(!face.IsActive())
                    {
                        if (face.Edge(0)->Id() == idEdge || face.Edge(1)->Id() == idEdge || face.Edge(2)->Id() == idEdge)
                        {
                            GenericFace& faceChild = *meshPointer->Face(face.Child(0)->Id());
                            unsigned int idEdgeChild = meshPointer->Edge(edge.Child(0)->Id())->Id();
                            if (faceChild.Edge(0)->Id() == idEdgeChild || faceChild.Edge(1)->Id() == idEdgeChild || faceChild.Edge(2)->Id() == idEdgeChild)
                                numFac++;
                            idEdgeChild = meshPointer->Edge(edge.Child(1)->Id())->Id();
                            if (faceChild.Edge(0)->Id() == idEdgeChild || faceChild.Edge(1)->Id() == idEdgeChild || faceChild.Edge(2)->Id() == idEdgeChild)
                                numFac++;
                        }
                    }
                }
                if (numFac > iCount)
                {
                    iCount = numFac;
                    ii = i;
                }
            }

            // Cerco il lato più lungo
            // A parità di lunghezza scelgo quello disattivo dei 2
            // Se ho due lati lunghi uguali entrambi disattivi scelgo quello con più facce attive

            Vector3d d = (edge.Point(0)->Coordinates()) - (edge.Point(1)->Coordinates());
            double lenght = d.dot(d);

            numFac = 0;
            for (unsigned int j = 0; j < 4; j++)
            {
                unsigned int idFace = cell.Face(j)->Id();
                GenericFace& face = *meshPointer->Face(idFace);
                if(face.IsActive())
                {
                    if (face.Edge(0)->Id() == idEdge || face.Edge(1)->Id() == idEdge || face.Edge(2)->Id() == idEdge)
                    {
                        numFac++;
                    }
                }
            }
            if (lenght > maxLenght || (lenght == maxLenght && !edge.IsActive() && (active || (!active && numFac > 0))))
            {
                maxLenght = lenght;
                il = i;
                active = edge.IsActive();
            }
        }

        ///scelta del criterio

        // Dopo aver scelto quale sia il lato da tagliore lo mettiamo in posizione 0 nel vettore dei lati della cella

        if(inactiveFaces == 2 && inactiveEdges == 1)
        {
            // Un lato inattivo e le due facce adiacenti tagliate lungo quel lato

            unsigned int idEdge = cell.Edge(ii)->Id();
            GenericEdge& edge = *meshPointer->Edge(idEdge);
            Vector3d d = (edge.Point(0)->Coordinates()) - (edge.Point(1)->Coordinates());
            double lenght = d.dot(d);
            if (toll*toll*lenght >= maxLenght)
            {
                // Se il tetraedro è ben proporzionato chiudo il taglio

                if (ii != 0)
                {
                    unsigned int idEdge = cell.Edge(0)->Id();
                    GenericEdge* edge = meshPointer->Edge(idEdge);
                    idEdge = cell.Edge(ii)->Id();

                    cell.InsertEdge(meshPointer->Edge(idEdge), 0);
                    cell.InsertEdge(edge, ii);
                }
            }
            else
            {
                // Altrimenti applico il criterio del lato più lungo

                if (il != 0)
                {
                    unsigned int idEdge = cell.Edge(0)->Id();
                    GenericEdge* edge = meshPointer->Edge(idEdge);
                    idEdge = cell.Edge(il)->Id();

                    cell.InsertEdge(meshPointer->Edge(idEdge), 0);
                    cell.InsertEdge(edge, il);
                }
            }
        }
        else if(inactiveFaces >= 2 && iCount == 2)
        {
            // Almeno un lato con le due facce adiacenti tagliate lungo quel lato
            // Sono presenti almeno due lati disattivi il che ci garantisce che le celle figlie verranno ricontrollate
            // Per questo non è necessario controllare che il tetraedro sia ben proporzionato e applichiamo direttamente il taglio a chiudere

            if (ii != 0)
            {
                unsigned int idEdge = cell.Edge(0)->Id();
                GenericEdge* edge = meshPointer->Edge(idEdge);
                idEdge = cell.Edge(ii)->Id();

                cell.InsertEdge(meshPointer->Edge(idEdge), 0);
                cell.InsertEdge(edge, ii);
            }
        }
        else
        {
            // Altrimenti applichiamo il criterio del lato più lungo

            if (il != 0)
            {
                unsigned int idEdge = cell.Edge(0)->Id();
                GenericEdge* edge = meshPointer->Edge(idEdge);
                idEdge = cell.Edge(il)->Id();

                cell.InsertEdge(meshPointer->Edge(idEdge), 0);
                cell.InsertEdge(edge, il);
            }
        }

        #ifdef DEBUG
        cout << "Id: " << cell.Id() << "\tInactive Edges: " << inactiveEdges << "\tInactive Edges: " << inactiveFaces << endl;
        #endif // DEBUG

        return Output::Success;
    }
    // ***************************************************************************
    const Output::ExitCodes RefinerTetra::CutTetra(GenericCell& cell)
    {
        ///dichiarazione variabili

        unsigned int idNewPoint;
        unsigned int idNewEdge[2];

        unsigned int idPoints[4];
        unsigned int idEdges[6];
        unsigned int idFaces[4];

        ///ordino i punti: idPoints = [positivo, negativo, piano0, piano1]

        idPoints[0] = cell.Edge(0)->Point(0)->Id();
        idPoints[1] = cell.Edge(0)->Point(1)->Id();

        unsigned int c = 0;

		for(unsigned int i = 0; i < 4; i++)
		{
			unsigned int idPoint = cell.Point(i)->Id();

			if (idPoint != idPoints[0] && idPoint != idPoints[1])
            {
                idPoints[2 + c] = idPoint;
                c++;
			}
		}

        ///ordino i lati: idEdges = [toCut, positivo0, positivo1, negativo0, negativo1, piano]

		for(unsigned int i = 0; i < 6; i++)
		{
			unsigned int idEdge = cell.Edge(i)->Id();
			GenericEdge& edge = *meshPointer->Edge(idEdge);

			if (i==0)
            {
                idEdges[0] = idEdge;
            }
            else if (edge.Point(0)->Id() == idPoints[0] || edge.Point(1)->Id() == idPoints[0])
            {
                if (edge.Point(0)->Id() == idPoints[2] || edge.Point(1)->Id() == idPoints[2])
                {
                    idEdges[1] = idEdge;
                }
                else
                {
                    idEdges[2] = idEdge;
                }
            }
            else if (edge.Point(0)->Id() == idPoints[1] || edge.Point(1)->Id() == idPoints[1])
            {
                if (edge.Point(0)->Id() == idPoints[2] || edge.Point(1)->Id() == idPoints[2])
                {
                    idEdges[3] = idEdge;
                }
                else
                {
                    idEdges[4] = idEdge;
                }
            }
            else
            {
                idEdges[5] = idEdge;
            }
        }

        ///ordino le facce: idFaces = [toCut0, toCut1, positiva, negativa]

		for(unsigned int i = 0; i < 4; i++)
		{
			unsigned int idFace = cell.Face(i)->Id();
			GenericFace& face = *meshPointer->Face(idFace);

			if(face.Edge(0)->Id() == idEdges[0] || face.Edge(1)->Id() == idEdges[0] || face.Edge(2)->Id() == idEdges[0])
            {
                if(face.Point(0)->Id() == idPoints[2] || face.Point(1)->Id() == idPoints[2] || face.Point(2)->Id() == idPoints[2])
                {
                    idFaces[0] = idFace;
                }
                else
                {
                    idFaces[1] = idFace;
                }
			}
            else if(face.Point(0)->Id() == idPoints[0] || face.Point(1)->Id() == idPoints[0] || face.Point(2)->Id() == idPoints[0])
            {
                idFaces[2] = idFace;
            }
            else
            {
                idFaces[3] = idFace;
            }
		}

		///taglio lato

        if (cell.Edge(0)->IsActive())
        {
			// Se il lato è attivo effettuo il taglio

			GenericEdge& edge = *meshPointer->Edge(idEdges[0]);
			GenericPoint& point0 = *meshPointer->Point(idPoints[0]);
			GenericPoint& point1 = *meshPointer->Point(idPoints[1]);

            edge.SetState(false);
            edge.InitializeChilds(2);

            ///creo nuovo punto

            Vector3d cutPoint = 0.5*(point0.Coordinates() + point1.Coordinates());
            GenericPoint& newPoint = *(meshPointer->CreatePoint());
            meshPointer->AddPoint(&newPoint);
            newPoint.SetCoordinates(cutPoint);
            idNewPoint = newPoint.Id();

            ///creo nuovi lati e aggiorno i punti

            GenericEdge& newEdge_0 = *(meshPointer->CreateEdge());
            meshPointer->AddEdge(& newEdge_0);
            newEdge_0.AddPoint(& point0);
            newEdge_0.AddPoint(& newPoint);
            edge.AddChild(& newEdge_0);
            newEdge_0.SetFather(& edge);
            newEdge_0.InheritPropertiesByFather();

            GenericEdge& newEdge_1 = *(meshPointer->CreateEdge());
            meshPointer->AddEdge(& newEdge_1);
            newEdge_1.AddPoint(& newPoint);
            newEdge_1.AddPoint(& point1);
            edge.AddChild(& newEdge_1);
            newEdge_1.SetFather(& edge);
            newEdge_1.InheritPropertiesByFather();

            newPoint.AddEdge(&newEdge_0);
            point0.AddEdge(&newEdge_0);
            newPoint.AddEdge(&newEdge_1);
            point1.AddEdge(&newEdge_1);

            ///aggiungo facce ai nuovi lati

            for(unsigned int fac = 0; fac < edge.NumberOfFaces(); fac++)
            {
                if(edge.Face(fac) != NULL)
                {
                    const GenericFace* faceTemp = edge.Face(fac);
                    newEdge_0.AddFace(faceTemp);
                    newEdge_1.AddFace(faceTemp);
                }
            }

            #ifdef DEBUG
            cout << "Id facce aggiunte tramite taglio del lato:";
            #endif // DEBUG

            ///aggiungo celle ai nuovi lati e le inserirsco tra le celle da aggiornare

            for(unsigned int cel = 0; cel < edge.NumberOfCells(); cel++)
            {
                if(edge.Cell(cel) != NULL && edge.Cell(cel)->IsActive())
                {
                    const GenericCell* cellTemp = edge.Cell(cel);
                    newEdge_0.AddCell(cellTemp);
                    newEdge_1.AddCell(cellTemp);

                    if(cellTemp->Id() != cell.Id())
                    {
                        idCellsToUpdate.insert(cellTemp->Id());

                        #ifdef DEBUG
                        cout << "\t" << cellTemp->Id();
                        #endif // DEBUG

                    }
                }
            }
            #ifdef DEBUG
            cout << endl;
            #endif // DEBUG
        }
        else
        {
            // Se il lato è inattivo vuol dire che è già stato tagliato
            // Prendo il punto creato e metto in ordine i figli: Child0: Positivo e Child1: Negativo

            unsigned int idChild0 = cell.Edge(0)->Child(0)->Id();
			GenericEdge* edgeChild0 = meshPointer->Edge(idChild0);

			if (edgeChild0->Point(0)->Id() == idPoints[0])
                idNewPoint = edgeChild0->Point(1)->Id();
			else if (edgeChild0->Point(1)->Id() == idPoints[0])
                idNewPoint = edgeChild0->Point(0)->Id();
			else
            {
                unsigned int idChild1 = cell.Edge(0)->Child(1)->Id();
                GenericEdge* edgeChild1 = meshPointer->Edge(idChild1);

                meshPointer->Edge(idEdges[0])->InsertChild(edgeChild1, 0);
                meshPointer->Edge(idEdges[0])->InsertChild(edgeChild0, 1);

                if (edgeChild1->Point(0)->Id() == idPoints[0])
                    idNewPoint = edgeChild1->Point(1)->Id();
                else
                    idNewPoint = edgeChild1->Point(0)->Id();
            }
        }

        ///taglio facce

        for (unsigned int i = 0; i < 2; i++)
        {
            GenericFace& face = *meshPointer->Face(idFaces[i]);

            if (face.IsActive())
            {
                // Se il lato è attivo effettuo il taglio

                GenericPoint& point0 = *meshPointer->Point(idNewPoint);
                GenericPoint& point1 = *meshPointer->Point(idPoints[2+i]);

                face.SetState(false);
                face.InitializeChilds(2);

                ///creo il nuovo lato e aggiorno i suoi punti

                GenericEdge& newEdge = *(meshPointer->CreateEdge());
                meshPointer->AddEdge(&newEdge);
                idNewEdge[i] = newEdge.Id();

                newEdge.AddPoint(&point0);
                newEdge.AddPoint(&point1);
                point0.AddEdge(&newEdge);
                point1.AddEdge(&newEdge);

                ///creo nuove facce

                GenericFace& newFace_0 = *(meshPointer->CreateFace());
                meshPointer->AddFace(&newFace_0);
                face.AddChild(&newFace_0);
                newFace_0.SetFather(&face);
                newFace_0.InheritPropertiesByFather();

                GenericFace& newFace_1 = *(meshPointer->CreateFace());
                meshPointer->AddFace(&newFace_1);
                face.AddChild(&newFace_1);
                newFace_1.SetFather(&face);
                newFace_1.InheritPropertiesByFather();

                ///aggiungo le celle alle nuove facce

                newFace_0.AllocateCells(2);
                newFace_0.InsertCell(face.Cell(0), 0);
                newFace_0.InsertCell(face.Cell(1), 1);

                newFace_1.AllocateCells(2);
                newFace_1.InsertCell(face.Cell(0), 0);
                newFace_1.InsertCell(face.Cell(1), 1);

                ///aggiorno i lati delle nuove facce

                newFace_0.AddEdge(&newEdge);
                newEdge.AddFace(&newFace_0);
                newFace_1.AddEdge(&newEdge);
                newEdge.AddFace(&newFace_1);

                GenericEdge* edge = meshPointer->Edge(idEdges[1+i]);
                newFace_0.AddEdge(edge);
                edge->AddFace(&newFace_0);
                edge = meshPointer->Edge(cell.Edge(0)->Child(0)->Id());
                newFace_0.AddEdge(edge);
                edge->AddFace(&newFace_0);

                edge = meshPointer->Edge(idEdges[3+i]);
                newFace_1.AddEdge(edge);
                edge->AddFace(&newFace_1);
                edge = meshPointer->Edge(cell.Edge(0)->Child(1)->Id());
                newFace_1.AddEdge(edge);
                edge->AddFace(&newFace_1);

                ///aggiorno i punti delle nuove faccie

                GenericPoint* point = meshPointer->Point(newEdge.Point(0)->Id());
                newFace_0.AddPoint(point);
                point->AddFace(&newFace_0);
                point = meshPointer->Point(newEdge.Point(1)->Id());
                newFace_0.AddPoint(point);
                point->AddFace(&newFace_0);
                point = meshPointer->Point(idPoints[0]);
                newFace_0.AddPoint(point);
                point->AddFace(&newFace_0);

                point = meshPointer->Point(newEdge.Point(0)->Id());
                newFace_1.AddPoint(point);
                point->AddFace(&newFace_1);
                point = meshPointer->Point(newEdge.Point(1)->Id());
                newFace_1.AddPoint(point);
                point->AddFace(&newFace_1);
                point = meshPointer->Point(idPoints[1]);
                newFace_1.AddPoint(point);
                point->AddFace(&newFace_1);

                ///computo le proprietą

                newFace_0.ComputeGeometricalProperties();
                newFace_1.ComputeGeometricalProperties();

                ///aggiungo le celle da aggiornare

                #ifdef DEBUG
                cout << "Id facce aggiunte tramite taglio delle faccia:";
                #endif // DEBUG

                for(unsigned int cel = 0; cel < face.NumberOfCells(); cel++)
                {
                    if(face.Cell(cel) != NULL && face.Cell(cel)->IsActive())
                    {
                        if(face.Cell(cel)->Id() != cell.Id())
                        {
                            idCellsToUpdate.insert(face.Cell(cel)->Id());

                            #ifdef DEBUG
                            cout << "\t" << face.Cell(cel)->Id();
                            #endif // DEBUG
                        }
                    }
                }
                #ifdef DEBUG
                cout << endl;
                #endif // DEBUG
            }
            else
            {
                // Se la faccia è inattiva vuol dire che è già stata tagliata
                // Prendo il lato creato e metto in ordine la facce figlie: Child0: Positiva e Child1: Negativa

                unsigned int idChild0 = face.Child(0)->Id();
                GenericFace* faceChild0 = meshPointer->Face(idChild0);

                for (unsigned int numEdge = 0; numEdge < 3; numEdge++)
                {
                    if (faceChild0->Edge(numEdge)->Point(0)->Id() == idNewPoint || faceChild0->Edge(numEdge)->Point(1)->Id() == idNewPoint)
                    {
                        if (faceChild0->Edge(numEdge)->Point(0)->Id() == idPoints[2+i] || faceChild0->Edge(numEdge)->Point(1)->Id() == idPoints[2+i])
                        {
                            idNewEdge[i] = faceChild0->Edge(numEdge)->Id();
                        }
                    }
                }

                if (faceChild0->Point(0)->Id() != idPoints[0] && faceChild0->Point(1)->Id() != idPoints[0] && faceChild0->Point(2)->Id() != idPoints[0])
                {
                    unsigned int idChild1 = face.Child(1)->Id();
                    GenericFace* faceChild1 = meshPointer->Face(idChild1);

                    face.InsertChild(faceChild1, 0);
                    face.InsertChild(faceChild0, 1);
                }
            }
        }

        ///creo nuova faccia

        GenericFace& newFace = *(meshPointer->CreateFace());
        meshPointer->AddFace(&newFace);

        if(newFace.InitializeProperty("OriginCell") == Output::Success)
        {
            const GenericTreeNode* node = &cell;
            while(node->HasFather())
                node = node->Father();
            newFace.AddProperty("OriginCell", new unsigned int(node->Id()));
        }

        ///aggiorno i lati della nuova faccia

        GenericEdge* edge = meshPointer->Edge(idNewEdge[1]);
        newFace.AddEdge(edge);
        edge->AddFace(&newFace);
        edge = meshPointer->Edge(idEdges[5]);
        newFace.AddEdge(edge);
        edge->AddFace(&newFace);
        edge = meshPointer->Edge(idNewEdge[0]);
        newFace.AddEdge(edge);
        edge->AddFace(&newFace);

        ///aggiorno i punti della nuova faccia

        GenericPoint* point = meshPointer->Point(idNewPoint);
        newFace.AddPoint(point);
        point->AddFace(&newFace);
        point = meshPointer->Point(idPoints[2]);
        newFace.AddPoint(point);
        point->AddFace(&newFace);
        point = meshPointer->Point(idPoints[3]);
        newFace.AddPoint(point);
        point->AddFace(&newFace);

        newFace.ComputeGeometricalProperties();

        ///creo le nuove celle

        cell.SetState(false);
        cell.InitializeChilds(2);

        GenericCell& newCell_0 = *(meshPointer->CreateCell());
        meshPointer->AddCell(&newCell_0);
        cell.AddChild(&newCell_0);

        newCell_0.SetFather(&cell);
        newCell_0.InheritPropertiesByFather();
        newCell_0.InitializeProperty("OriginCell");
        if(cell.HasProperty("OriginCell"))
            newCell_0.AddProperty("OriginCell", cell.GetProperty("OriginCell"));
        else
        {
            const GenericTreeNode* node = &cell;
            while(node->HasFather())
                node = node->Father();
            newCell_0.AddProperty("OriginCell", new unsigned int(node->Id()));
        }

        GenericCell& newCell_1 = *(meshPointer->CreateCell());
        meshPointer->AddCell(&newCell_1);
        cell.AddChild(&newCell_1);
        newCell_1.SetFather(&cell);
        newCell_1.InheritPropertiesByFather();
        newCell_1.InitializeProperty("OriginCell");

        if(cell.HasProperty("OriginCell"))
            newCell_1.AddProperty("OriginCell", cell.GetProperty("OriginCell"));
        else
            newCell_1.AddProperty("OriginCell", new int(cell.Id()));

        ///aggiorno le facce e le celle vicine delle nuove celle

        unsigned int positionCell = 1;
        newCell_0.InitializeCells(4);

        newCell_0.AddFace(&newFace);
        newCell_0.InsertCell(&newCell_1, 0);
        newFace.AddCell(&newCell_0);

        GenericFace* face = meshPointer->Face(idFaces[2]);
        newCell_0.AddFace(face);
        if(face->Cell(0)->Id() == cell.Id())
        {
            newCell_0.InsertCell(face->Cell(1), positionCell);
            face->InsertCell(&newCell_0, 0);
        }
        else
        {
            newCell_0.InsertCell(face->Cell(0), positionCell);
            face->InsertCell(&newCell_0, 1);
        }
        positionCell++;

        face = meshPointer->Face(idFaces[0]);
        face = meshPointer->Face(face->Child(0)->Id());
        newCell_0.AddFace(face);
        if(face->Cell(0)->Id() == cell.Id())
        {
            newCell_0.InsertCell(face->Cell(1), positionCell);
            face->InsertCell(&newCell_0, 0);
        }
        else
        {
            newCell_0.InsertCell(face->Cell(0), positionCell);
            face->InsertCell(&newCell_0, 1);
        }
        positionCell++;

        face = meshPointer->Face(idFaces[1]);
        face = meshPointer->Face(face->Child(0)->Id());
        newCell_0.AddFace(face);
        if(face->Cell(0)->Id() == cell.Id())
        {
            newCell_0.InsertCell(face->Cell(1), positionCell);
            face->InsertCell(&newCell_0, 0);
        }
        else
        {
            newCell_0.InsertCell(face->Cell(0), positionCell);
            face->InsertCell(&newCell_0, 1);
        }

        positionCell = 1;
        newCell_1.InitializeCells(4);

        newCell_1.AddFace(&newFace);
        newCell_1.InsertCell(&newCell_0, 0);
        newFace.AddCell(&newCell_1);

        face = meshPointer->Face(idFaces[3]);
        newCell_1.AddFace(face);
        if(face->Cell(0)->Id() == cell.Id())
        {
            newCell_1.InsertCell(face->Cell(1), positionCell);
            face->InsertCell(&newCell_1, 0);
        }
        else
        {
            newCell_1.InsertCell(face->Cell(0), positionCell);
            face->InsertCell(&newCell_1, 1);
        }
        positionCell++;

        face = meshPointer->Face(idFaces[0]);
        face = meshPointer->Face(face->Child(1)->Id());
        newCell_1.AddFace(face);
        if(face->Cell(0)->Id() == cell.Id())
        {
            newCell_1.InsertCell(face->Cell(1), positionCell);
            face->InsertCell(&newCell_1, 0);
        }
        else
        {
            newCell_1.InsertCell(face->Cell(0), positionCell);
            face->InsertCell(&newCell_1, 1);
        }
        positionCell++;

        face = meshPointer->Face(idFaces[1]);
        face = meshPointer->Face(face->Child(1)->Id());
        newCell_1.AddFace(face);
        if(face->Cell(0)->Id() == cell.Id())
        {
            newCell_1.InsertCell(face->Cell(1), positionCell);
            face->InsertCell(&newCell_1, 0);
        }
        else
        {
            newCell_1.InsertCell(face->Cell(0), positionCell);
            face->InsertCell(&newCell_1, 1);
        }

        ///aggiorno lati delle nuove celle

        edge = meshPointer->Edge(idNewEdge[0]);
        newCell_0.AddEdge(edge);
        newCell_1.AddEdge(edge);
        edge->AddCell(&newCell_0);
        edge->AddCell(&newCell_1);

        edge = meshPointer->Edge(idNewEdge[1]);
        newCell_0.AddEdge(edge);
        newCell_1.AddEdge(edge);
        edge->AddCell(&newCell_0);
        edge->AddCell(&newCell_1);

        edge = meshPointer->Edge(idEdges[5]);
        newCell_0.AddEdge(edge);
        newCell_1.AddEdge(edge);
        edge->AddCell(&newCell_0);
        edge->AddCell(&newCell_1);

        edge = meshPointer->Edge(idEdges[1]);
        newCell_0.AddEdge(edge);
        edge->AddCell(&newCell_0);

        edge = meshPointer->Edge(idEdges[2]);
        newCell_0.AddEdge(edge);
        edge->AddCell(&newCell_0);

        edge = meshPointer->Edge(idEdges[0]);
        edge = meshPointer->Edge(edge->Child(0)->Id());
        newCell_0.AddEdge(edge);
        edge->AddCell(&newCell_0);

        edge = meshPointer->Edge(idEdges[3]);
        newCell_1.AddEdge(edge);
        edge->AddCell(&newCell_1);

        edge = meshPointer->Edge(idEdges[4]);
        newCell_1.AddEdge(edge);
        edge->AddCell(&newCell_1);

        edge = meshPointer->Edge(idEdges[0]);
        edge = meshPointer->Edge(edge->Child(1)->Id());
        newCell_1.AddEdge(edge);
        edge->AddCell(&newCell_1);

        ///aggiorno i punti delle nuove celle

        point = meshPointer->Point(idNewPoint);
        newCell_0.AddPoint(point);
        newCell_1.AddPoint(point);
        point->AddCell(&newCell_0);
        point->AddCell(&newCell_1);

        point = meshPointer->Point(idPoints[2]);
        newCell_0.AddPoint(point);
        newCell_1.AddPoint(point);
        point->AddCell(&newCell_0);
        point->AddCell(&newCell_1);

        point = meshPointer->Point(idPoints[3]);
        newCell_0.AddPoint(point);
        newCell_1.AddPoint(point);
        point->AddCell(&newCell_0);
        point->AddCell(&newCell_1);

        point = meshPointer->Point(idPoints[0]);
        newCell_0.AddPoint(point);
        point->AddCell(&newCell_0);

        point = meshPointer->Point(idPoints[1]);
        newCell_1.AddPoint(point);
        point->AddCell(&newCell_1);

        ///metto le celle figlie tra le celle da controllare

        idCellsToUpdate.insert(cell.Child(0)->Id());
        idCellsToUpdate.insert(cell.Child(1)->Id());

        return Output::Success;
    }
    // ***************************************************************************
    const Output::ExitCodes RefinerTetra::RecoverConformity()
    {
        while(!idCellsToUpdate.empty())
        {
            set<unsigned int >::iterator id = idCellsToUpdate.begin();
            GenericCell& cell = *meshPointer->Cell(*id);

            if (!cell.IsActive())
            {
                idCellsToUpdate.erase(cell.Id());
                idCellsToUpdate.insert(cell.Child(0)->Id());
                idCellsToUpdate.insert(cell.Child(1)->Id());
            }
            else
            {
                unsigned int inactiveEdges = 0;

                for(unsigned int i = 0; i<6; i++)
                {
                    unsigned int idEdge = cell.Edge(i)->Id();
                    GenericEdge& edge = *meshPointer->Edge(idEdge);
                    if(!edge.IsActive())
                        inactiveEdges++;
                }

                // Se il numero di lati inattivi è pari a 0 vuol dire che la cella è conforme con le vicine

                if (inactiveEdges != 0)
                {
                    // Se la cella non è conforme viene effettuato un altro taglio
                    // Modificando il sencondo parametro di FindMaxEdge() è possibile imporre limiti sulle proporzioni della cella
                    // Se questi limiti sono rispettati si può applicare il criterio semplificativo

                    FindMaxEdge(cell, 2);
                    CutTetra(cell);

                    #ifdef DEBUG
                    cout << "First Child: " << cell.Child(0)->Id() << "\tSecond Child: " << cell.Child(1)->Id() << endl;
                    #endif // DEBUG

                }

                idCellsToUpdate.erase(cell.Id());
            }
        }

        return Output::Success;
     }
    // ***************************************************************************
    const Output::ExitCodes RefinerTetra::AddIdCell(const unsigned int& idCell)
	{
		idCellToRefine.push_back(idCell);
		return Output::Success;
	}
    // ***************************************************************************
	const Output::ExitCodes RefinerTetra::RefineMesh()
	{
        for(unsigned int numCell = 0; numCell < idCellToRefine.size(); numCell++)
        {
            // Vado a tagliare tutte le celle che sono state inserite in idCellToRefine

            GenericCell* cell = meshPointer->Cell(idCellToRefine[numCell]);

            if(!cell->IsActive())
            {
                const GenericTreeNode* cellChild = cell->Child(0);
                while(cellChild->HasChilds())
                    cellChild = cellChild->Child(0);

                unsigned int idChild = cellChild->Id();
                cell = meshPointer->Cell(idChild);
            }

            // In questo caso non è previsto un criterio semplificativo: viene sempre applicato il criterio del lato più lungo

            FindMaxEdge(*cell, 1);
            CutTetra(*cell);

            #ifdef DEBUG
            cout << "First Child: " << cell->Child(0)->Id() << "\tSecond Child: " << cell->Child(1)->Id() << endl;
            #endif // DEBUG

        }

        RecoverConformity();

		return Output::Success;
	}
    // ***************************************************************************
}
