./station-server StationA 4001 4002 localhost:4008 localhost:4010 localhost:4012 &
./station-server TerminalB 4003 4004 localhost:4006 &
./station-server JunctionC 4005 4006 localhost:4004 localhost:4008 &
./station-server BusportD 4007 4008 localhost:4002 localhost:4006 &
./station-server StationE 4009 4010 localhost:4002 localhost:4012 &
./station-server TerminalF 4011 4012 localhost:4002 localhost:4010 &