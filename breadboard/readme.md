# Breadboard pour Oric

![RENDU](images/breadboard.png "Rendu5")


# Schéma de l'adaptateur
![sch](schematics/breadboard_sch.png "sch")

# Adresses du 74LS138

| Sorties | Adresse début | Adresse fin |
|---------|--------------|------------|
| Y0      | $380         | $38F       |
| Y1      | $390         | $39F       |
| Y2      | $3A0         | $3AF       |
| Y3      | $3B0         | $3BF       |
| Y4      | $3C0         | $3CF       |
| Y5      | $3D0         | $3DF       |
| Y6      | $3E0         | $3EF       |
| Y7      | $3F0         | $3FF       |

# Test avec un PIA 6821
![6821](images/6821.jpg "6821")

PORT PA0 en sortie, PORT B en entrée
Clignote la led sur PA0

```console
 10 POKE #381,0
 20 POKE #380,1
 30 POKE #381,4
 40 POKE #380,0
 50 WAIT 10
 60 POKE #380,1
 70 WAIT 10
 80 GOTO 40
```


# Test avec un PIA 8255
![8255](images/8255.jpg "8255")

![sch](schematics/pia.png "8255")

PORT A en sortie, PORT B en entrée , PORT C en entrée 
Clignote la led sur PA0

```console
 10 POKE #383,#8B
 20 POKE #380,1
 30 WAIT 10
 40 POKE #380,0
 50 WAIT 10
 60 GOTO 20
```

