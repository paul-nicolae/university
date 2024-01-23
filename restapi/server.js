'use strict';
const express = require('express');
const bodyParser = require('body-parser');
const { Client } = require('pg');

// Constants
const PORT = 3000;
const HOST = '0.0.0.0';

// App
const app = express();

// parse application/json
app.use(bodyParser.json());

const client = new Client({
  user: 'postgres',
  host: 'postgres',
  database: 'postgres',
  password: 'postgres',
  port: 5432,
});

client.connect()

client.query('DROP TABLE IF EXISTS country; CREATE TABLE IF NOT EXISTS country (id SERIAL PRIMARY KEY, nume_tara VARCHAR(255) UNIQUE, latitudine FLOAT, longitudine FLOAT)');
client.query('DROP TABLE IF EXISTS city; CREATE TABLE IF NOT EXISTS city (id SERIAL PRIMARY KEY, id_tara INTEGER, nume_oras VARCHAR(255), latitudine FLOAT, longitudine FLOAT, UNIQUE(id_tara, nume_oras))');
client.query('DROP TABLE IF EXISTS temperature; CREATE TABLE IF NOT EXISTS temperature (id SERIAL PRIMARY KEY, valoarea FLOAT, timestamp TIMESTAMP DEFAULT NOW(), id_oras INTEGER, UNIQUE(timestamp, id_oras))');

app.post('/api/countries', (req, res) => {
  const { nume, lat, lon } = req.body;

  if (!nume || !lat || !lon) return res.status(400).send({message: 'Nu ati trimis toate datele'});

  // check if value of name is a string
  if (typeof nume !== 'string') return res.status(400).send({message: 'Numele trebuie sa fie un string'});

  // check if value of lat of lon is a number
  if (typeof lat !== 'number' || typeof lon !== 'number') return res.status(400).send({message: 'Latitudinea si longitudinea trebuie sa fie numere'});
  
  client.query('INSERT INTO country(nume_tara, latitudine, longitudine) VALUES($1, $2, $3) RETURNING *', [nume, lat, lon], (err, result) => {
    if (err) return res.status(409).send(err);
    res.status(201).send({id: result.rows[0].id});
  });
});
 
function parseCountries(rows) {
  return rows.map(row => ({
    id: row.id,
    nume: row.nume_tara,
    lat: row.latitudine,
    lon: row.longitudine
  }));
}

app.get('/api/countries', (req, res) => {
  client.query('SELECT * FROM country', (err, result) => {
    // return the result.rows in the schema format
    res.status(200).send(parseCountries(result.rows));
  });
});

app.post('/api/cities', (req, res) => {
  const { idTara, nume, lat, lon } = req.body;

  if (!idTara || !nume || !lat || !lon) return res.status(400).send({message: 'Nu ati trimis toate datele'});

  // check if the idTara exists
  client.query('SELECT * FROM country WHERE id = $1', [idTara], (err, result) => {
    if(result.rows.length === 0) return res.status(404).send({message: 'Tara nu exista'});

    client.query('INSERT INTO city(id_tara, nume_oras, latitudine, longitudine) VALUES($1, $2, $3, $4) RETURNING *', [idTara, nume, lat, lon], (err, result) => {
      if (err) return res.status(409).send(err);
      res.status(201).send({id: result.rows[0].id});
    });
  });
 
});

function parseCities(rows) {
  return rows.map(row => ({
    id: row.id,
    idTara: row.id_tara,
    nume: row.nume_oras,
    lat: row.latitudine,
    lon: row.longitudine
  }));
}

app.get('/api/cities', (req, res) => {
  client.query('SELECT * FROM city', (err, result) => {
    res.status(200).send(parseCities(result.rows));
  });
});

app.get('/api/cities/country/:id_Tara', (req, res) => {
  const { id_Tara } = req.params;

  // check if the idTara exists
  client.query('SELECT * FROM country WHERE id = $1 ', [id_Tara], (err, result) => {
    if (err) return res.status(404).send(err);
    if (result.rows.length === 0) return res.status(404).send({message: 'Tara nu exista'});

    client.query('SELECT * FROM city WHERE id_tara = $1', [id_Tara], (err, result) => {
      if (err) return res.status(404).send(err);
      if (result.rows.length === 0) return res.status(404).send({message: 'Nu exista orase in tara respectiva'});
      res.status(200).send(parseCities(result.rows));
    });
  });
});

app.post('/api/temperatures', (req, res) => {
  const { idOras, valoare } = req.body;

  // check if valoare is a number
  if (typeof valoare !== 'number') return res.status(400).send({message: 'Valoarea trebuie sa fie un numar'});

  // check if we have idOras or valoare
  if (!idOras || !valoare) return res.status(400).send({message: 'Nu ati trimis toate datele'});

  // check if the idOras exists
  client.query('SELECT * FROM city WHERE id = $1', [idOras], (err, result) => {
    if(result.rows.length === 0) return res.status(404).send({message: 'Orasul nu exista'});

    client.query('INSERT INTO temperature(id_oras, valoarea) VALUES($1, $2) RETURNING *', [idOras, valoare], (err, result) => {
      if (err) return res.status(409).send(err);
      res.status(201).send({id: result.rows[0].id});
    });
  });
});

const get_date = (timestamp) => {
  const date = new Date(timestamp);
  return date.getFullYear() + '-' + (date.getMonth() + 1) + '-' + date.getDate();
}

function parseTemperatures(rows) {
  return rows.map(row => ({
    id: row.id,
    valoare: row.valoarea,
    timestamp: get_date(row.timestamp)
  }));
}

app.get('/api/temperatures', (req, res) => {
  let { lat, lon, from, until } = req.query;

  if (!from) from = '1970-01-01';
  if (!until) until = '2100-01-01';
  let query = "SELECT t.id, t.valoarea, t.timestamp FROM temperature t JOIN city c ON c.id = t.id_oras WHERE timestamp >= '" + from + "'::DATE and timestamp <= '" + until + "'::DATE";

  if (lon) query = query + " and longitudine = " + lon;
  if (lat) query = query + " and latitudine = " + lat;
  
  client.query(query, (err, result) => {
    res.status(200).send(parseTemperatures(result.rows));
  });
});

app.get('/api/temperatures/cities/:idOras', (req, res) => {
  const { idOras } = req.params;
  let { from, until } = req.query;
  
  if (!from) from = '1970-01-01';
  if (!until) until = '2100-01-01';
  let query = "SELECT t.id, t.valoarea, t.timestamp FROM temperature t JOIN city c ON c.id = t.id_oras WHERE timestamp >= '" + from + "'::DATE and timestamp <= '" + until + "'::DATE and id_oras = " + idOras;

  client.query(query, (err, result) => {
    if (err) return res.status(404).send(err);
    res.status(200).send(parseTemperatures(result.rows));
  });
});

app.get('/api/temperatures/countries/:idTara', (req, res) => {
  const { idTara } = req.params;
  let { from, until } = req.query;

  if (!from) from = '1970-01-01';
  if (!until) until = '2100-01-01';
  let query = "SELECT t.id, t.valoarea, t.timestamp FROM temperature t JOIN city c ON c.id = t.id_oras WHERE timestamp >= '" + from + "'::DATE and timestamp <= '" + until + "'::DATE and id_tara = " + idTara;

  client.query(query, (err, result) => {
    if (err) return res.status(404).send(err);
    res.status(200).send(parseTemperatures(result.rows));
  });
});

app.put('/api/temperatures/:id', (req, res) => {
  const { id } = req.params;
  const { idBody, idOras, valoare } = req.body;

  if (idBody !== id) return res.status(400).send({message: 'Id-ul din body nu corespunde cu id-ul din url'});

  // check if the idOras exists
  client.query('SELECT * FROM city WHERE id = $1', [idOras], (err, result) => {
    if(result.rows.length === 0) return res.status(404).send({message: 'Orasul nu exista'});

    client.query('UPDATE temperature SET id_oras = $1, valoarea = $2 WHERE id = $3', [idOras, valoare, id], (err, result) => {
      if (err) return res.status(409).send(err);
      res.status(200).send();
    });
  });
});

app.delete('/api/temperatures/:id', (req, res) => {
  const { id } = req.params;

  client.query('DELETE FROM temperature WHERE id = $1', [id], (err, result) => {
    if (result.rowCount === 0) return res.status(404).send({message: 'Temperatura nu exista'});
    res.status(200).send();
  }); 
});

app.put('/api/cities/:id', (req, res) => {
  const { id } = req.params;
  const { idBody, idTara, nume, lat, lon } = req.body;

  if (idBody !== id) return res.status(400).send({message: 'Id-ul din body nu corespunde cu id-ul din url'});

  // check if the idTara exists
  client.query('SELECT * FROM country WHERE id = $1', [idTara], (err, result) => {
    if(result.rows.length === 0) return res.status(404).send({message: 'Tara nu exista'});

    client.query('UPDATE city SET id_tara = $1, nume_oras = $2, latitudine = $3, longitudine = $4 WHERE id = $5', [idTara, nume, lat, lon, id], (err, result) => {
      if (err) return res.status(409).send(err);
      res.status(200).send();
    });
  });
});

app.delete('/api/cities/:id', (req, res) => {
  const { id } = req.params;

  client.query('DELETE FROM city WHERE id = $1', [id], (err, result) => {
    if (result.rowCount === 0) return res.status(404).send({message: 'Orasul nu exista'});
    res.status(200).send();
  }); 
});

app.put('/api/countries/:id', (req, res) => {
  const { id } = req.params;
  const { idBody, nume, lat, lon } = req.body;

  if (idBody !== id) return res.status(400).send({message: 'Id-ul din body nu corespunde cu id-ul din url'});

  client.query('UPDATE country SET nume_tara = $1, latitudine = $2, longitudine = $3 WHERE id = $4', [nume, lat, lon, id], (err, result) => {
    if (err) return res.status(404).send(err);
    res.status(200).send();
  });
});

app.delete('/api/countries/:id', (req, res) => {
  const { id } = req.params;

  client.query('DELETE FROM country WHERE id = $1', [id], (err, result) => {
    if (result.rowCount === 0) return res.status(404).send({message: 'Tara nu exista'});
    res.status(200).send();
  }); 
});

app.listen(PORT, HOST);
console.log(`Running on http://${HOST}:${PORT}`);
