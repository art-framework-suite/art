
class ToyMCTrack{

public:
  ToyMCTrack ( int pdgid, double q[4], double pos[3] );
  virtual ~ToyMCTrack();

  const double* p()  const { return _p;}
  double px() const { return _p[0];}
  double py() const { return _p[1];}
  double pz() const { return _p[2];}
  double  e() const { return _p[3];}

private:

  double _p[4];
  double _pos[3];
  int   _pdgid;

};

