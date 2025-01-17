/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2019 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/

#ifndef __BITPIT_CELL_HPP__
#define __BITPIT_CELL_HPP__

#include <memory>

#include "bitpit_containers.hpp"

#include "element.hpp"

namespace bitpit {
	class Cell;
	class PatchKernel;
}

bitpit::IBinaryStream& operator>>(bitpit::IBinaryStream &buf, bitpit::Cell& cell);
bitpit::OBinaryStream& operator<<(bitpit::OBinaryStream &buf, const bitpit::Cell& cell);

namespace bitpit {

class Cell : public Element {

friend class PatchKernel;

friend bitpit::OBinaryStream& (::operator<<) (bitpit::OBinaryStream& buf, const Cell& cell);
friend bitpit::IBinaryStream& (::operator>>) (bitpit::IBinaryStream& buf, Cell& cell);

public:
	Cell();
	Cell(long id, ElementType type,
	     bool interior = true, bool storeNeighbourhood = true);
	Cell(long id, ElementType type, int connectSize,
	     bool interior = true, bool storeNeighbourhood = true);
	Cell(long id, ElementType type, std::unique_ptr<long[]> &&connectStorage,
	     bool interior = true, bool storeNeighbourhood = true);

	Cell(const Cell &other) = default;
	Cell(Cell&& other) = default;
	Cell& operator = (const Cell &other) = default;
	Cell& operator=(Cell&& other) = default;

	void swap(Cell &other) noexcept;

	void initialize(long id, ElementType type, bool interior, bool storeNeighbourhood = true);
	void initialize(long id, ElementType type, int connectSize, bool interior, bool storeNeighbourhood = true);
	void initialize(long id, ElementType type, std::unique_ptr<long[]> &&connectStorage, bool interior, bool storeNeighbourhood = true);

	bool isInterior() const;
	
	void deleteInterfaces();
	void resetInterfaces(bool storeInterfaces = true);
	void setInterfaces(std::vector<std::vector<long>> &interfaces);
	void setInterface(int face, int index, long interface);
	void pushInterface(int face, long interface);
	void deleteInterface(int face, int i);
	int getInterfaceCount() const;
	int getInterfaceCount(int face) const;
	long getInterface(int face, int index = 0) const;
	const long * getInterfaces() const;
	long * getInterfaces();
	const long * getInterfaces(int face) const;
	long * getInterfaces(int face);
	int findInterface(int face, int interface);
	int findInterface(int interface);

	void deleteAdjacencies();
	void resetAdjacencies(bool storeAdjacencies = true);
	void setAdjacencies(std::vector<std::vector<long>> &adjacencies);
	void setAdjacency(int face, int index, long adjacencies);
	void pushAdjacency(int face, long adjacency);
	void deleteAdjacency(int face, int i);
	int getAdjacencyCount() const;
	int getAdjacencyCount(int face) const;
	long getAdjacency(int face, int index = 0) const;
	const long * getAdjacencies() const;
	long * getAdjacencies();
	const long * getAdjacencies(int face) const;
	long * getAdjacencies(int face);
	int findAdjacency(int face, int adjacency);
	int findAdjacency(int adjacency);

	bool isFaceBorder(int face) const;

	void display(std::ostream &out, unsigned short int indent) const;

	unsigned int getBinarySize() const;

protected:
	void setInterior(bool interior);

private:
	bool m_interior;

	bitpit::FlatVector2D<long> m_interfaces;
	bitpit::FlatVector2D<long> m_adjacencies;

	bitpit::FlatVector2D<long> createNeighbourhoodStorage(bool storeNeighbourhood);

	void _initialize(bool interior, bool initializeNeighbourhood, bool storeNeighbourhood);

};

class CellHalfFace : public ElementHalfFace {

public:
	CellHalfFace(Cell &cell, int face, Winding winding = WINDING_NATURAL);

	Cell & getCell() const;

protected:
	using ElementHalfFace::getElement;

};

extern template class PiercedVector<Cell>;

}

#endif
