#include <gtest/gtest.h>
#include <fstream>

#include "MusicLibrary/filesystemmusiclibrary.h"
#include "Core/playqueue.h"
#include "Core/settings.h"

using namespace std;
using namespace Gejengel;

class PlayQueueSubscriberMock : public PlayQueueSubscriber
{
public:
    void onTrackQueued(uint32_t index) { queuedIndexes.push_back(index); }
    void onTrackRemoved(uint32_t index) { removedIndexes.push_back(index); }
    void onTrackMoved(uint32_t sourceIndex, uint32_t destIndex) { movedFromIndexes.push_back(sourceIndex); movedToIndexes.push_back(destIndex); }
    void onQueueCleared() { queueCleared = true; }

    std::vector<uint32_t> queuedIndexes;
    std::vector<uint32_t> removedIndexes;
    std::vector<uint32_t> movedFromIndexes;
    std::vector<uint32_t> movedToIndexes;
    bool queueCleared;
};

class PlayQueueTest : public testing::Test
{
protected:
    PlayQueueTest()
    : library(settings)
    , q(library)
    {
    }
    
    virtual void SetUp()
    {
        q.subscribe(qSub);
    }

    Settings                settings;
    FilesystemMusicLibrary  library;
    PlayQueue               q;
    PlayQueueSubscriberMock qSub;
};

TEST_F(PlayQueueTest, AddTrack)
{
    Track track1, track2, track3, track4, track5;
    track1.id = 1; track2.id = 2; track3.id = 3; track4.id = 4; track5.id = 5;
    
    q.queueTrack(track1);
    q.queueTrack(track2);
    q.queueTrack(track3, 0);
    q.queueTrack(track4, 45); //should be added to the back
    q.queueTrack(track5, 2);

    ASSERT_EQ(5, q.size());

    Track track;
    //assert queue is now 3, 1, 5, 2, 4
    q.getNextTrack(track);
    ASSERT_EQ(track3, track);
    q.getNextTrack(track);
    ASSERT_EQ(track1, track);
    q.getNextTrack(track);
    ASSERT_EQ(track5, track);
    q.getNextTrack(track);
    ASSERT_EQ(track2, track);
    q.getNextTrack(track);
    ASSERT_EQ(track4, track);

    ASSERT_EQ(0, q.size());

    ASSERT_EQ(5, qSub.queuedIndexes.size());
    ASSERT_EQ(0, qSub.queuedIndexes[0]);
    ASSERT_EQ(1, qSub.queuedIndexes[1]);
    ASSERT_EQ(0, qSub.queuedIndexes[2]);
    ASSERT_EQ(3, qSub.queuedIndexes[3]);
    ASSERT_EQ(2, qSub.queuedIndexes[4]);
}

TEST_F(PlayQueueTest, RemoveTrack)
{
    Track track1, track2, track3;
    track1.id = 1; track2.id = 2; track3.id = 3;
    
    q.queueTrack(track1);
    q.queueTrack(track2);
    q.queueTrack(track3);

    q.removeTrack(1);

    ASSERT_EQ(1, qSub.removedIndexes.size());
    ASSERT_EQ(1, qSub.removedIndexes[0]);
    ASSERT_EQ(2, q.size());

    Track track;
    //assert queue is now 1, 3
    q.getNextTrack(track);
    ASSERT_EQ(track1, track);
    q.getNextTrack(track);
    ASSERT_EQ(track3, track);

    ASSERT_EQ(3, qSub.removedIndexes.size());
    ASSERT_EQ(0, qSub.removedIndexes[1]);
    ASSERT_EQ(0, qSub.removedIndexes[2]);
}

TEST_F(PlayQueueTest, RemoveTracks)
{
    Track track1, track2, track3, track4, track5;
    track1.id = 1; track2.id = 2; track3.id = 3; track4.id = 4; track5.id = 5;
    
    q.queueTrack(track1);
    q.queueTrack(track2);
    q.queueTrack(track3);
    q.queueTrack(track4);
    q.queueTrack(track5);

    std::vector<uint32_t> indexes;
    indexes.push_back(4);
    indexes.push_back(0);
    indexes.push_back(2);

    q.removeTracks(indexes);

    ASSERT_EQ(3, qSub.removedIndexes.size());
    ASSERT_EQ(0, qSub.removedIndexes[0]);
    ASSERT_EQ(2-1, qSub.removedIndexes[1]);
    ASSERT_EQ(4-2, qSub.removedIndexes[2]);
    ASSERT_EQ(2, q.size());

    Track track;
    //assert queue is now 2, 4
    q.getNextTrack(track);
    ASSERT_EQ(track2, track);
    q.getNextTrack(track);
    ASSERT_EQ(track4, track);
}

TEST_F(PlayQueueTest, MoveTrack)
{
    Track track1, track2, track3;
    track1.id = 1; track2.id = 2; track3.id = 3;

    q.queueTrack(track1);
    q.queueTrack(track2);
    q.queueTrack(track3);

    q.moveTrack(0, 2); //2, 3, 1
    q.moveTrack(1, 0); //3, 2, 1

    ASSERT_EQ(2, qSub.movedFromIndexes.size());
    ASSERT_EQ(2, qSub.movedToIndexes.size());

    ASSERT_EQ(0, qSub.movedFromIndexes[0]);
    ASSERT_EQ(1, qSub.movedFromIndexes[1]);

    ASSERT_EQ(2, qSub.movedToIndexes[0]);
    ASSERT_EQ(0, qSub.movedToIndexes[1]);

    ASSERT_EQ(3, q.size());

    Track track;
    //assert queue is now 3, 2, 1
    q.getNextTrack(track);
    ASSERT_EQ(track3, track);
    q.getNextTrack(track);
    ASSERT_EQ(track2, track);
    q.getNextTrack(track);
    ASSERT_EQ(track1, track);
}
