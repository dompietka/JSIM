CREATE TABLE statek (
  nazwa VARCHAR2(24) PRIMARY KEY,
  ladownosc NUMBER NOT NULL
);

CREATE TABLE port (
  nazwa VARCHAR2(20) PRIMARY KEY
);

CREATE TABLE rejs (
  numer NUMBER PRIMARY KEY,
  skad VARCHAR2(20) NOT NULL REFERENCES port,
  dokad VARCHAR2(20) NOT NULL REFERENCES port,
  poczatek NUMBER NOT NULL,
  koniec NUMBER NOT NULL,
  statek VARCHAR2(24) NOT NULL REFERENCES statek
);

CREATE TABLE manifest (
  rejs NUMBER NOT NULL REFERENCES rejs,
  towar VARCHAR2(20) NOT NULL,
  ilosc NUMBER NOT NULL
);

column nazwa format a24
column statek format a24
column skad format a20
column dokad format a20
column towar format a20
set linesize 200

INSERT INTO statek VALUES ('Weeping Somnambulist', 100);
INSERT INTO statek VALUES ('Perishable Harvest', 100);
INSERT INTO statek VALUES ('Eight Tenants of Bushido', 400);
INSERT INTO statek VALUES ('Malaclypse', 200);

INSERT INTO port VALUES ('Ceres');
INSERT INTO port VALUES ('Ganymede');
INSERT INTO port VALUES ('Bara Gaon');
INSERT INTO port VALUES ('Auberon');
INSERT INTO port VALUES ('Kronos');
INSERT INTO port VALUES ('Laconia');


INSERT INTO rejs VALUES (100, 'Bara Gaon', 'Auberon', 1500, 1700, 'Malaclypse');
INSERT INTO rejs VALUES (110, 'Ganymede', 'Kronos', 1910, 1990, 'Weeping Somnambulist');
INSERT INTO rejs VALUES (111, 'Laconia', 'Kronos', 1900, 2000, 'Eight Tenants of Bushido');
INSERT INTO rejs VALUES (112, 'Bara Gaon', 'Kronos', 1900, 2001, 'Perishable Harvest');
INSERT INTO rejs VALUES (113, 'Ceres', 'Kronos', 1900, 2003, 'Malaclypse');
INSERT INTO rejs VALUES (220, 'Kronos', 'Ceres', 1995, 2100, 'Weeping Somnambulist');
INSERT INTO rejs VALUES (222, 'Kronos', 'Bara Gaon', 2010, 2101, 'Perishable Harvest');
INSERT INTO rejs VALUES (333, 'Kronos', 'Auberon', 2002, 2202, 'Eight Tenants of Bushido');
INSERT INTO rejs VALUES (440, 'Ceres', 'Auberon', 2101, 2199, 'Weeping Somnambulist');
INSERT INTO rejs VALUES (444, 'Bara Gaon', 'Laconia', 2110, 2200, 'Perishable Harvest');



INSERT INTO manifest VALUES (110, 'pietruszka', 10);
INSERT INTO manifest VALUES (110, 'marchewka', 10);
INSERT INTO manifest VALUES (110, 'ziemniaki', 20);
INSERT INTO manifest VALUES (110, 'cebula', 10);
INSERT INTO manifest VALUES (110, 'ogorki', 10);
INSERT INTO manifest VALUES (110, 'czosnek', 5);
INSERT INTO manifest VALUES (110, 'baranina', 10);
INSERT INTO manifest VALUES (110, 'wino', 10);
INSERT INTO manifest VALUES (110, 'musztarda', 10);

INSERT INTO manifest VALUES (111, 'konserwy', 400);
INSERT INTO manifest VALUES (112, 'olej', 100);
INSERT INTO manifest VALUES (113, 'elektronika', 200);
INSERT INTO manifest VALUES (220, 'lit', 100);
INSERT INTO manifest VALUES (222, 'lit', 100);
INSERT INTO manifest VALUES (333, 'lit', 200);
INSERT INTO manifest VALUES (440, 'elektronika', 100);
INSERT INTO manifest VALUES (444, 'rzodkiewka', 100);

