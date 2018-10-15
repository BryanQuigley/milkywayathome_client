--
-- Copyright (C) 2011  Matthew Arsenault
--
-- This file is part of Milkway@Home.
--
-- Milkyway@Home is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.

-- Milkyway@Home is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Milkyway@Home.  If not, see <http://www.gnu.org/licenses/>.
--
--

require "NBodyTesting"

local SamplePotentials = { }

SamplePotentials.buildAllDisks = function()
   local noDisks, miyamotoNagaiDisks, doubleExponentialDisks, sech2ExponentialDisks, freemanDisks
   noDisks = buildAllCombinations(
      function(m)
         return Disk.none{ mass = m }
      end,
      { 4.5e5, 5.0e6, 3.0e4 }
   )

   miyamotoNagaiDisks = buildAllCombinations(
      function(m, a, b)
         return Disk.miyamotoNagai{ mass = m, scaleLength = a, scaleHeight = b }
      end,
      { 4.5e5, 5.0e6, 3.0e4 },
      { 6.5, 9.0, 3.0 },
      { 0.26, 0.5, 0.1 }
   )

   doubleExponentialDisks = buildAllCombinations(
      function(m, a, b)
         return Disk.exponential{ mass = m, scaleLength = a, scaleHeight = b }
      end,
      { 2.24933e5, 3.0e6, 1.5e4 },
      { 3.6, 2.0, 6.5 },
      { 0.26, 0.5, 0.1}
   )

   sech2ExponentialDisks = buildAllCombinations(
      function(m, a, b)
         return Disk.exponential{ mass = m, scaleLength = a, scaleHeight = b }
      end,
      { 2.24933e5, 3.0e6, 1.5e4 },
      { 3.6, 2.0, 6.5 },
      { 0.26, 0.5, 0.1}
   )

   freemanDisks = buildAllCombinations(
      function(m, a)
         return Disk.exponential{ mass = m, scaleLength = a }
      end,
      { 2.24933e5, 3.0e6, 1.5e4 },
      { 3.6, 2.0, 6.5 }
   )

   return mergeTables(noDisks, miyamotoNagaiDisks, doubleExponentialDisks, sech2ExponentialDisks, freemanDisks)
end

SamplePotentials.buildAllDisk2s = function()
   local noDisk2s, miyamotoNagaiDisk2s, doubleExponentialDisk2s, sech2ExponentialDisk2s, freemanDisk2s
   noDisk2s = buildAllCombinations(
      function(m)
         return Disk2.none{ mass = m }
      end,
      { 4.5e5, 5.0e6, 3.0e4 }
   )

   miyamotoNagaiDisk2s = buildAllCombinations(
      function(m, a, b)
         return Disk2.miyamotoNagai{ mass = m, scaleLength = a, scaleHeight = b }
      end,
      { 4.5e5, 5.0e6, 3.0e4 },
      { 6.5, 9.0, 3.0 },
      { 0.26, 0.5, 0.1 }
   )

   doubleExponentialDisk2s = buildAllCombinations(
      function(m, a, b)
         return Disk2.exponential{ mass = m, scaleLength = a, scaleHeight = b }
      end,
      { 2.24933e5, 3.0e6, 1.5e4 },
      { 3.6, 2.0, 6.5 },
      { 0.26, 0.5, 0.1}
   )

   sech2ExponentialDisk2s = buildAllCombinations(
      function(m, a, b)
         return Disk2.exponential{ mass = m, scaleLength = a, scaleHeight = b }
      end,
      { 2.24933e5, 3.0e6, 1.5e4 },
      { 3.6, 2.0, 6.5 },
      { 0.26, 0.5, 0.1}
   )

   freemanDisk2s = buildAllCombinations(
      function(m, a)
         return Disk2.exponential{ mass = m, scaleLength = a }
      end,
      { 2.24933e5, 3.0e6, 1.5e4 },
      { 3.6, 2.0, 6.5 }
   )

   return mergeTables(noDisk2s, miyamotoNagaiDisk2s, doubleExponentialDisk2s, sech2ExponentialDisk2s, freemanDisk2s)
end

SamplePotentials.buildAllSphericals = function()
   local noSphericals, herquistSphericals, plummerSphericals
   noSphericals = buildAllCombinations(
      function(m)
         return Spherical.none{ mass = m }
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 }
   )

   herquistSphericals = buildAllCombinations(
      function(m, s)
         return Spherical.herquist{ mass = m, scale = s }
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 },
      { 0.7, 1.1, 0.4 }
   )

   plummerSphericals = buildAllCombinations(
      function(m, s)
         return Spherical.plummer{ mass = m, scale = s }
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 },
      { 0.7, 1.1, 0.4 }
   )

   return mergeTables(noSphericals, herquistSphericals, plummerSphericals)
end


SamplePotentials.buildAllHalos = function()
   local noHalos, logHalos, nfwHalos, triaxialHalos, ASHalos, WEHalos, nfwmHalos, plummerHalos, hernquistHalos, ninkovicHalos

   noHalos = buildAllCombinations(
      function(m)
         return Halo.none{ mass = m }
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 }
   )

   logHalos = buildAllCombinations(
      function(v, L, fZ)
         return Halo.logarithmic{ vhalo = v, scaleLength = L, flattenZ = fZ }
      end,
      { 73, 100, 50 },
      { 12, 20, 6 },
      { 1, 1.4, 0.7 }
   )

   nfwHalos = buildAllCombinations(
      function(v, L)
         return Halo.nfw{ vhalo = v, scaleLength = L }
      end,
      { 155, 200, 120 },
      { 22.25, 30, 14 }
   )

   triaxialHalos = buildAllCombinations(
      function(v, L, fZ, fX, fY, angle)
         return Halo.triaxial{ vhalo = v, scaleLength = L,
                               flattenZ = fZ, flattenX = fX, flattenY = fY,
                               triaxAngle = angle
                            }
      end,
      { 116, 150, 87 },
      { 16.3, 20, 12 },
      { 1.43, 1.7, 1.2 },
      { 1.26, 1.5, 1.1 },
      { 1.0, 1.2, 0.7 },
      { 96, 120, 66 }
   )

   ASHalos = buildAllCombinations(
      function(m, L, l, g)
         return Halo.allenSantillan{ mass = m, scaleLength = L, lambda = l, gamma = g}
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 },
      { 16.3, 20, 12 },
      { 100.0, 200.0, 50.0},
      { 2.0, 3.0, 4.0 }
   )

   WEHalos = buildAllCombinations(
      function(m, L)
         return Halo.wilkinsonEvans{ mass = m, scaleLength = L }
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 },
      { 16.3, 20, 12 }
   )

   nfwmHalos = buildAllCombinations(
      function(m, L)
         return Halo.nfwmass{ mass = m, scaleLength = L }
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 },
      { 16.3, 20, 12 }
   )

   plummerHalos = buildAllCombinations(
      function(m, L)
         return Halo.plummer{ mass = m, scaleLength = L }
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 },
      { 16.3, 20, 12 }
   )

   hernquistHalos = buildAllCombinations(
      function(m, L)
         return Halo.hernquist{ mass = m, scaleLength = L }
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 },
      { 16.3, 20, 12 }
   )

   ninkovicHalos = buildAllCombinations(
      function(p, L, l)
         return Halo.ninkovic{ rho0 = p, scaleLength = L, lambda = l }
      end,
      { 1.52954402e5, 1.7e6, 1.1e4 },
      { 16.3, 20, 12 },
      { 70.0, 96.5, 150.0}
   )

   return mergeTables(noHalos, logHalos, nfwHalos, triaxialHalos, ASHalos, WEHalos, nfwmHalos, plummerHalos, hernquistHalos, ninkovicHalos)
end

SamplePotentials.makeAllPotentials = function()
  return buildAllCombinations(Potential.create,
                              SamplePotentials.buildAllSphericals(),
                              SamplePotentials.buildAllDisks(),
                              SamplePotentials.buildAllDisk2s(),
                              SamplePotentials.buildAllHalos())

end

-- Artificial tests, should have at least every combination of
-- spherical, disk, halo component typs. (750 COMBINATIONS!?!)
SamplePotentials.samplePotentials = {
   potentialA = Potential.create{
      spherical = Spherical.spherical{
         mass  = 1.5e5,
         scale = 0.7
      },

      disk = Disk.miyamotoNagai{
         mass        = 4.5e5,
         scaleLength = 6.0,
         scaleHeight = 0.3
      },

      halo = Halo.logarithmic{
         vhalo       = 80,
         scaleLength = 15,
         flattenZ    = 1.0
      }
   },

   potentialB = Potential.create{
      spherical = Spherical.spherical{
         mass  = 2.0e5,
         scale = 0.7
      },

      disk = Disk.exponential{
         mass        = 9.0e5,
         scaleLength = 6
      },

      halo = Halo.logarithmic{
         vhalo       = 60,
         scaleLength = 10.0,
         flattenZ    = 1.1
      }
   },

   potentialC = Potential.create{
      spherical = Spherical.spherical{
         mass  = 2.0e6,
         scale = 1.0
      },

      disk = Disk.exponential{
         scaleLength = 9,
         mass        = 9.0e5
      },

      halo = Halo.nfw{
         vhalo       = 150,
         scaleLength = 25
      }
   },

   potentialD = Potential.create{
      spherical = Spherical.spherical{
         mass  = 2.0e6,
         scale = 1.0
      },

      disk = Disk.miyamotoNagai{
         mass        = 2e6,
         scaleLength = 7.0,
         scaleHeight = 0.2
      },

      halo = Halo.triaxial{
         vhalo       = 120,
         scaleLength = 18,
         flattenZ = 1.45,
         flattenX = 1.3,
         flattenY = 1.0,
         triaxAngle = 96
      }
   },

   potentialE = Potential.create{
      spherical = Spherical.spherical{
         mass  = 2e5,
         scale = 0.4
      },

      disk = Disk.miyamotoNagai{
         mass        = 7e5,
         scaleLength = 9.0,
         scaleHeight = 0.5
      },

      halo = Halo.nfw{
         vhalo       = 120,
         scaleLength = 18,
      }
   },

   potentialF = Potential.create{
      spherical = Spherical.spherical{
         mass  = 2.0e6,
         scale = 1.0
      },

      disk = Disk.exponential{
         mass        = 3.0e7,
         scaleLength = 8,
      },

      halo = Halo.triaxial{
         vhalo       = 140,
         scaleLength = 25,
         flattenZ = 1.3,
         flattenX = 1.4,
         flattenY = 1.1,
         triaxAngle = 112
      }
   }
}

SamplePotentials.samplePotentialNames = getKeyNames(SamplePotentials.samplePotentials)

SamplePotentials.randomHalo = function(prng)
   if prng == nil then
      prng = DSFMT.create()
   end

   local typ = floor(prng:random(0, 10))

   if typ == 0 then
      return Halo.none{
         mass        = prng:random(1.0e4, 5.0e7)
      }
   elseif typ == 1 then
      return Halo.logarithmic{
         vhalo       = prng:random(10, 200),
         scaleLength = prng:random(3, 100),
         flattenZ    = prng:random(0, 3)
      }
   elseif typ == 2 then
      return Halo.nfw{
         vhalo       = prng:random(1, 300),
         scaleLength = prng:random(0.1, 40)
      }
   elseif typ == 3 then
      return Halo.triaxial{
         vhalo       = prng:random(1, 200),
         scaleLength = prng:random(0.1, 30),
         flattenX    = prng:random(0, 4),
         flattenY    = prng:random(0, 3),
         flattenZ    = prng:random(0, 4),
         triaxAngle  = prng:random(0, 180)
      }
   elseif typ == 4 then
      return Halo.allenSantillan{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(0.1, 30),
         lambda      = prng:random(20.0, 300.0),
         gamma       = prng:random(2.0, 5.0)
      }
   elseif typ == 5 then
      return Halo.wilkinsonEvans{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(0.1, 30)
      }
   elseif typ == 6 then
      return Halo.nfwmass{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(0.1, 30)
      }
   elseif typ == 7 then
      return Halo.plummer{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(0.1, 30)
      }
   elseif typ == 8 then
      return Halo.hernquist{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(0.1, 30)
      }
   elseif typ == 9 then
      return Halo.ninkovic{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(0.1, 30),
         lambda      = prng:random(30.0, 150.0)
      }
   else
      assert(false)
   end
end

SamplePotentials.randomDisk = function(prng)
   if prng == nil then
      prng = DSFMT.create()
   end

   local typ = floor(prng:random(0, 5))

   if typ == 0 then
      return Disk.none{
         mass        = prng:random(1.0e4, 5.0e7)
      }
   elseif typ == 1 then
      return Disk.miyamotoNagai{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(1, 10),
         scaleHeight = prng:random(0.1, 8)
      }
   elseif typ == 2 then
      return Disk.doubleExponential{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(1, 10),
         scaleHeight = prng:random(0.1, 8)
      }
   elseif typ == 3 then
      return Disk.sech2Exponential{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(1, 10),
         scaleHeight = prng:random(0.1, 8)
      }
   elseif typ == 4 then
      return Disk.freeman{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(1, 10)
      }
   else
      assert(false)
   end
end

SamplePotentials.randomDisk2 = function(prng)
   if prng == nil then
      prng = DSFMT.create()
   end

   local typ = floor(prng:random(0, 5))

   if typ == 0 then
      return Disk2.none{
         mass        = prng:random(1.0e4, 5.0e7)
      }
   elseif typ == 1 then
      return Disk2.miyamotoNagai{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(1, 10),
         scaleHeight = prng:random(0.1, 8)
      }
   elseif typ == 2 then
      return Disk2.doubleExponential{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(1, 10),
         scaleHeight = prng:random(0.1, 8)
      }
   elseif typ == 3 then
      return Disk2.sech2Exponential{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(1, 10),
         scaleHeight = prng:random(0.1, 8)
      }
   elseif typ == 4 then
      return Disk2.freeman{
         mass        = prng:random(1.0e4, 5.0e7),
         scaleLength = prng:random(1, 10)
      }
   else
      assert(false)
   end
end

SamplePotentials.randomSpherical = function(prng)
   if prng == nil then
      prng = DSFMT.create()
   end

   local typ == floor(prng:random(0, 3))

   if typ == 0 then
      return Spherical.none{
         mass        = prng:random(1.0e4, 5.0e7)
      }
   elseif typ == 1 then
      return Spherical.hernquist{
         mass  = prng:random(1.0e4, 9e7),
         scale = prng:random(0, 4)
      }
   elseif typ == 2 then
      return Spherical.plummer{
         mass  = prng:random(1.0e4, 9e7),
         scale = prng:random(0, 4)
      }
   else
      assert(false)
   end
end

SamplePotentials.randomPotential = function(prng)
   return Potential.create{
      spherical = SamplePotentials.randomSpherical(prng),
      disk      = SamplePotentials.randomDisk(prng),
      disk2     = SamplePotentials.randomDisk2(prng),
      halo      = SamplePotentials.randomHalo(prng)
   }
end


return SamplePotentials


