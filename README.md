# Table of Contents

- [Project Management](#project-management)
- [Battery Life](#battery-life)

# Project Management

## Init

```bash
mkdir project-nrf-dishwasher-sign
cd project-nrf-dishwasher-sign
docker run --rm -u $(id -u):$(id -g) -v $(pwd):/new -w /new zephyrprojectrtos/ci:v0.26.6 \
        bash -c "west init -m https://github.com/fgervais/project-nrf-dishwasher-sign.git . && west update"
```

## Build

```bash
cd application
docker compose run --rm nrf west build -b pink_panda -s app
```

## menuconfig

```bash
cd application
docker compose run --rm nrf west build -b pink_panda -s app -t menuconfig
```

## Clean

```bash
cd application
rm -rf build/
```

## Update

```bash
cd application
docker compose run --rm nrf west update
```

## Flash

### nrfjprog
```bash
cd application
docker compose -f docker-compose.yml -f docker-compose.device.yml \
        run nrf west flash
```

### pyocd
```bash
cd application
pyocd flash -e sector -t nrf52840 -f 4000000 build/zephyr/zephyr.hex
```

# Battery Life

## Power consumption

- 3.3v
- Internal DC/DC converter

### Idle

Average: 2.28µA

The code wakes up every 3 minutes to feed the watchdog.

![Idle Consumption](assets/img/idle-consumption-dcdc.png)

### Screen refresh

31.64mC

![Screen Refresh](assets/img/screen-refresh.png)

## Expected life (Energizer L91 AA Ultimate Lithium)

3500 mAh = 12600 Coulomb

As the usage consumption is so low compared to the battery capacity, the
limiting factor is basically the self discharge of the cell.

The self discharge on those primary cells is quite low so the batteries are
expected to last a long time.

### Idle all the time

```math
\frac{12600 \, C}{2.28 \times 10^-6 \, \frac{C}{s}} = 5.5 \times 10^9 \, s
```

```math
5.5 \times 10^9 \, seconds \times \frac{1 \, minute}{60 \, second} \times \frac{1 \, hour}{60 \, minute} \times \frac{1 \, day}{24 \, hour} \times \frac{1 \, year}{365.25 \, days} = 175 \, years
```

### Refreshing all the time

```math
12600 \, C \div 233.14 \times 10^-3 \, \frac{C}{day} = 3474 \, days = 9.51 \, years

\frac{12600 \, C}{31.64 \times 10^-3 \, C} = 398 \times 10^-3 \, refreshes
```

Note that the project will refresh the screen twice a day to comply with the
display guidelines.

On 10 years, that would be about 7305 refreshes.
 