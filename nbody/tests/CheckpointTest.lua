--
-- Copyright (C) 2011  Matthew Arsenault
--
-- This file is part of Milkway@Home.
--
-- Milkyway@Home is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
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
SM = require "SampleModels"
SP = require "SamplePotentials"

function erf(x)       --Pulled from https://hewgill.com/picomath/lua/erf.lua.html
    -- constants
    a1 =  0.254829592
    a2 = -0.284496736
    a3 =  1.421413741
    a4 = -1.453152027
    a5 =  1.061405429
    p  =  0.3275911

    -- Save the sign of x
    sign = 1
    if x < 0 then
        sign = -1
    end
    x = math.abs(x)

    -- A&S formula 7.1.26
    t = 1.0/(1.0 + p*x)
    y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*math.exp(-x*x)

    return sign*y
end

function randomNBodyCtx(prng)
   if prng == nil then
      prng = DSFMT.create()
   end
   sigma = prng:random(1.5,3.0)
   correct = math.sqrt(2*3.1415926535)/(math.sqrt(2*3.1415926535)*erf(sigma/math.sqrt(2)) - 2*sigma*math.exp(-sigma*sigma/2))
   return NBodyCtx.create{
      timestep      = prng:random(1.0e-5, 1.0e-4),
      timeEvolve    = prng:random(0, 10),
      theta         = prng:random(0, 1),
      eps2          = prng:random(1.0e-9, 1.0e-3),
      orbit_parameter_b  = 53.5,
      orbit_parameter_r  = 28.6,
      orbit_parameter_vx = -156,
      orbit_parameter_vy = 79,
      orbit_parameter_vz = 107,
      treeRSize     = prng:randomListItem({ 4, 8, 2, 16 }),
      criterion     = prng:randomListItem({"TreeCode", "SW93", "BH86", "Exact"}),
      useQuad       = prng:randomBool(),
      BestLikeStart = prng:random(0.85,0.99),
      BetaSigma     = sigma,
      VelSigma      = sigma,
      DistSigma     = sigma,
      BetaCorrect   = correct,
      VelCorrect    = correct,
      DistCorrect   = correct,
      IterMax       = floor(prng:random(2,10)),
      allowIncest   = true,
      quietErrors   = true
   }
end

function runNSteps(st, n, ctx)
   for i = 1, n do
      st:step(ctx)
   end
   return ctx, st
end

function runInterruptedSteps(st, totalSteps, ctx, prng)
   local tmpDir = os.getenv("TMP") or ""
   local checkpoint = tmpDir .. os.tmpname()

   for i = 1, totalSteps do
      st:step(ctx)
      if prng:randomBool() then
         local tmp = tmpDir .. os.tmpname()
         st:writeCheckpoint(ctx, checkpoint, tmp)
         ctx, st = NBodyState.readCheckpoint(checkpoint)
         os.remove(checkpoint)
      end
   end

   return ctx, st
end


local nTests = 5

for i = 1, nTests do
   local testSteps, st, stCopy
   local ctx, m
   local prng = DSFMT.create()

   m = SM.randomPlummer(prng, 500)
   ctx = randomNBodyCtx(prng)
   ctx:addPotential(SP.randomPotential(prng))


   st = NBodyState.create(ctx, m)
   stClone = st:clone()

   testSteps = floor(prng:random(0, 51))

   ctx, st = runNSteps(st, testSteps, ctx)
   ctxClone, stClone = runInterruptedSteps(stClone, testSteps, ctx, prng)

   assert(ctx == ctxClone,
          string.format("Checkpointed context does not match:\nctx 1 = %s\n ctx 2 = %s\n",
                        tostring(ctx),
                        tostring(ctxClone))
       )

   assert(st == stClone,
          string.format("Checkpointed state does not match:\nstate 1 = %s\n state 2 = %s\n",
                        tostring(st),
                        tostring(stClone))
       )
end


