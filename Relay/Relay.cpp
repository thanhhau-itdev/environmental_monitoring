#include "Relay.h"

Relay::Relay(uint8_t pin, bool activeLevel)
{
  _pin = pin;
  _activeLevel = activeLevel;
  _state = false;
}

void Relay::begin()
{
  pinMode(_pin, OUTPUT);
  off();
}

void Relay::on()
{
  digitalWrite(_pin, _activeLevel);
  _state = true;
}

void Relay::off()
{
  digitalWrite(_pin, !_activeLevel);
  _state = false;
}

void Relay::toggle()
{
  _state ? off() : on();
}

bool Relay::state()
{
  return _state;
}
