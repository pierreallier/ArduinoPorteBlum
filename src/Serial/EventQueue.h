#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include <Arduino.h>
#include "../Config/Codes.h"
#include "../Config/Types.h"



/**
 * @brief File circulaire des messages en attente d'envoi.
 *
 * Capacité fixe. Si la file est pleine, le nouveau message écrase le plus ancien
 * (le compteur perdus() est alors incrémenté).
 *
 * Non appelable depuis une interruption (pas de protection).
 * RAM utilisée : CAPACITE x 6 octets.
 */
class EventQueue {
  public:
    static constexpr uint8_t CAPACITE = 20;   // à ajuster selon la RAM disponible

    static EventQueue& instance() {
        static EventQueue q;
        return q;
    }

    // Retourne true si aucun message n'a été écrasé.
    bool push(char type, MSG code, int32_t val = 0) {
        bool ecrase = (_count == CAPACITE);
        if (ecrase) {                        // file pleine : on supprime le plus ancien
            _head = (_head + 1) % CAPACITE;
            _count--;
            _perdus++;
        }
        Event& e = _buf[(_head + _count) % CAPACITE];
        e.type = type;
        e.code = code;
        e.val  = val;
        _count++;
        return !ecrase;
    }

    bool empty() const { return _count == 0; }
    bool clearPerdu() const {return _perdus == 0;}
    uint8_t size() const { return _count; }
    uint16_t perdus() const { return _perdus; }

    const Event& peek() const { return _buf[_head]; }   // appeler seulement si !empty()

    void pop() {
        if (_count == 0) return;
        _head = (_head + 1) % CAPACITE;
        _count--;
    }

  private:
    EventQueue() = default;
    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;

    Event    _buf[CAPACITE];
    uint8_t  _head = 0;
    uint8_t  _count = 0;
    uint16_t _perdus = 0;
};

#endif