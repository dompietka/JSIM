SET LINESIZE 300;
COLUMN NAME FORMAT a6;
COLUMN CONTENTS FORMAT a72;

DROP TABLE Challenge;
DROP TABLE Puzzle;
DROP TABLE Player;

-- Gracze

CREATE TABLE Player (
  name VARCHAR2(10) PRIMARY KEY
);

-- Zagadki

CREATE TABLE Puzzle (
  id NUMBER PRIMARY KEY,
  contents VARCHAR2(100) NOT NULL,
  author VARCHAR2(10) NOT NULL REFERENCES Player
);

-- Wyzwania

CREATE TABLE Challenge (
  puzzle NUMBER NOT NULL REFERENCES Puzzle,
  sender VARCHAR2(10) NOT NULL REFERENCES PLayer,
  receiver VARCHAR2(10) NOT NULL REFERENCES Player,
  received DATE NOT NULL,
  solved DATE,
  PRIMARY KEY (puzzle, sender, receiver)
);

INSERT INTO Player VALUES ('Ania');
INSERT INTO Player VALUES ('Bartek');
INSERT INTO Player VALUES ('Czesia');
INSERT INTO Player VALUES ('Dzidek');

INSERT INTO Puzzle VALUES (1,'Jaka jest najbardziej krolewska czesc trojkata? - Jego wysokosc.','Ania');
INSERT INTO Puzzle VALUES (2,'Ile zebow ma dentysta? - Sto ma, to logiczne.','Ania');
INSERT INTO Puzzle VALUES (3,'Ktory miesiac ma 28 dni? - Kazdy.','Czesia');


INSERT INTO Challenge VALUES (1,'Ania','Bartek','01-FEB-2024','01-FEB-2024');
INSERT INTO Challenge VALUES (1,'Ania','Czesia','01-FEB-2024','02-FEB-2024');
INSERT INTO Challenge VALUES (1,'Ania','Dzidek','01-FEB-2024','01-FEB-2024');
INSERT INTO Challenge VALUES (2,'Ania','Bartek','02-FEB-2024','02-FEB-2024');
INSERT INTO Challenge VALUES (2,'Ania','Czesia','02-FEB-2024',NULL);
INSERT INTO Challenge VALUES (3,'Ania','Bartek','05-FEB-2024','05-FEB-2024'); 
INSERT INTO Challenge VALUES (2,'Ania','Dzidek','02-FEB-2024','04-FEB-2024');
INSERT INTO Challenge VALUES (3,'Czesia','Bartek','04-FEB-2024','05-FEB-2024'); 
INSERT INTO Challenge VALUES (3,'Bartek','Dzidek','05-FEB-2024','06-FEB-2024');

COMMIT;

