#include "Chunk.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "Block/Block.h"
#include "Texture/Texture_Atlas.h"
#include "Loader.h"
#include "Random.h"
#include "Block/D_Blocks.h"
#include "Chunk_Map.h"
#include "Debug_Display.h"
#include "Noise_Generator.h"

void Chunk::generateBlockData()
{
    auto maxHeight = 0;
    std::vector<int> m_heightMap;
    for (int x = 0; x < SIZE ; x ++)
    {
        for (int z = 0 ; z < SIZE ; z++)
        {
            auto height =  Noise_Generator::getHeight (x,
                                                        z,
                                                        m_location.x,
                                                        m_location.z );
            if (height > maxHeight) maxHeight = height;
            m_heightMap.push_back(height);
        }
    }

    if(maxHeight <= WATER_LEVEL) maxHeight = WATER_LEVEL + 1;

    for (int y = 0; y < maxHeight + 1 ; y++)
    {
        for (int x = 0 ; x < SIZE ; x++)
        {
            for (int z = 0 ; z < SIZE ; z++)
            {
                int h = m_heightMap.at (x * SIZE + z);
                if (y > h)
                {
                    y <= WATER_LEVEL ?
                        qSetBlock({x, y, z}, Block::water) :
                        qSetBlock({x, y, z}, Block::air);
                }
                else if (y == h)
                {
                    if (y > BEACH_LEVEL) //Top levels
                    {
                        if ( y <= SNOW_LEVEL )
                        {
                            qSetBlock({x, y, z}, Block::grass );
                            if ( Random::integer(0, y) == 1  &&
                               (x > 3 && x < SIZE - 3) &&
                               (z > 3 && z < SIZE - 3)
                                && y <= SNOW_LEVEL - 10)
                            {
                                m_treeLocations.emplace_back(x, y, z);    //Trees
                            }
                        }
                        else
                        {
                            Random::integer(y, maxHeight + 10) < y + 5?
                                qSetBlock({x, y, z}, Block::snowGrass) :
                                qSetBlock({x, y, z}, Block::grass );
                        }

                    }
                    else if (y <= BEACH_LEVEL && y >= WATER_LEVEL) //Beach
                    {
                        qSetBlock({x, y, z}, Block::sand);
                    }
                    else
                    {
                        Random::integer(0, 10) < 6 ?
                            qSetBlock({x, y, z}, Block::sand)   :
                            qSetBlock({x, y, z}, Block::dirt);
                    }
                }
                else  if (y < h && y > h - 5)
                {
                    qSetBlock({x, y, z}, Block::dirt);
                }
                else
                {
                    qSetBlock({x, y, z}, Block::stone);
                }
            }
        }
    }
}

void Chunk::generateStructureData ()
{
    for (auto& location : m_treeLocations)
    {
        makeTree(location);
    }
    m_treeLocations.clear();
}

void Chunk::loadBlockData ()
{
    std::ifstream inFile(getFileString());

    if(!inFile.is_open())
        return;

    int x, y, z, id;

    while(inFile.peek() != EOF)
    {
        inFile >> x >> y >> z >> id;
        m_addedBlocks[{x, y, z}] = id;
    }

    for (auto& block : m_addedBlocks)
    {
        int idNum = block.second;
        Block::ID id = static_cast<Block::ID>(idNum);

        qSetBlock(block.first, Block::getBlockFromId(id));
    }
}

void Chunk::generateMesh ()
{
    m_p_chunkMap->addChunk({m_location.x + 1, m_location.z});
    m_p_chunkMap->addChunk({m_location.x, m_location.z + 1});
    m_p_chunkMap->addChunk({m_location.x - 1, m_location.z});
    m_p_chunkMap->addChunk({m_location.x, m_location.z - 1});

    m_mesh.generateMesh(m_layers.size());

    m_hasMesh       = true;
    m_hasBuffered   = false;
}

void Chunk::bufferMesh ()
{
    m_mesh.bufferMesh();

    m_hasBuffered = true;
}

void Chunk::makeTree (const Block_Location& location)
{
    auto trunkHeight = Random::integer(5, 8);
    //Make the trunk
    for (auto i = 1 ; i < trunkHeight + 1 ; i++)
    {
        qSetBlock({location.x, location.y + i, location.z}, Block::oakWood, false);
    }
    //Make the crown
    for (auto yLeaf = location.y + trunkHeight ; yLeaf < location.y + trunkHeight + 4 ; yLeaf++)
    {
        for (auto xLeaf = location.x - 2 ; xLeaf < location.x + 3 ; xLeaf++)
        {
            for (auto zLeaf = location.z - 2 ; zLeaf < location.z + 3 ; zLeaf++)
            {
                qSetBlock({xLeaf, yLeaf, zLeaf}, Block::oakLeaf, false );
            }
        }
    }
}
