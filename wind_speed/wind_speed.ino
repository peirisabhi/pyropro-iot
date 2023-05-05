volatile byte revolutions;

float rpmilli;
float speed;


unsigned long timeold=0;

void setup()
{
  Serial.begin(9600);
  Serial.print("Started");
  attachInterrupt(digitalPinToInterrupt(27), rpm_fun, RISING);

  revolutions = 0;
  rpmilli = 0;
  timeold = 0;
}

void loop()
{
  if (revolutions >= 2) { 
    //Update RPM every 20 counts, increase this for better RPM resolution,
    //decrease for faster update

    // calculate the revolutions per milli(second)
    rpmilli = ((float)revolutions)/(millis()-timeold);

    timeold = millis();
    revolutions = 0;

    // WHEELCIRC = 2 * PI * radius (in meters)
    // speed = rpmilli * WHEELCIRC * "milliseconds per hour" / "meters per kilometer"

    // simplify the equation to reduce the number of floating point operations
    // speed = rpmilli * WHEELCIRC * 3600000 / 1000
    // speed = rpmilli * WHEELCIRC * 3600

    speed = rpmilli * 0.6597 * 3600;

    Serial.print("RPM:");
    Serial.print(rpmilli * 60000);
    Serial.print(" Speed:");
    Serial.print(speed);
    Serial.println(" kph");
  }
}

void rpm_fun()
{
  revolutions++;
}