
class ToyMCHit{

public:
  ToyMCHit ( int track_id,
	     int channel_id,
	     double distance,
	     double pulse_height);
  virtual ~ToyMCHit();

  int trackId()        const { return track;}
  int channelID()      const { return chan;}
  double distance()    const { return dist;}
  double pulseHeight() const { return ph;}


private:

  // Index of track that made this hit.
  int track;

  // Channel ID.
  int chan;

  // Whatever it is we measure.
  double dist;
  double ph;



};

