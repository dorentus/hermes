/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

(function(){
class Vec2D {
    x: number;
    y: number;

    constructor(x: number, y: number) {
        this.x = Math.sqrt(10);
        this.y = y;
    }
}

function dotProduct(a: Vec2D, b: Vec2D) {
    return a.x*b.x + a.y*b.y;
}

return [dotProduct, Vec2D];
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): object
// CHECK-NEXT:  %1 = CallInst (:object) %0: object, %""(): object, empty: any, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function ""(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %dotProduct(): number
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %Vec2D(): undefined
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2: object, %1: object, "prototype": string
// CHECK-NEXT:  %4 = AllocFastArrayInst (:object) 2: number
// CHECK-NEXT:  %5 = FastArrayPushInst %0: object, %4: object
// CHECK-NEXT:  %6 = FastArrayPushInst %1: object, %4: object
// CHECK-NEXT:  %7 = ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function dotProduct(a: any, b: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %2 = PrLoadInst (:number) %0: any, 0: number, "x": string
// CHECK-NEXT:  %3 = PrLoadInst (:number) %1: any, 0: number, "x": string
// CHECK-NEXT:  %4 = BinaryMultiplyInst (:number) %2: number, %3: number
// CHECK-NEXT:  %5 = PrLoadInst (:number) %0: any, 1: number, "y": string
// CHECK-NEXT:  %6 = PrLoadInst (:number) %1: any, 1: number, "y": string
// CHECK-NEXT:  %7 = BinaryMultiplyInst (:number) %5: number, %6: number
// CHECK-NEXT:  %8 = BinaryAddInst (:number) %4: number, %7: number
// CHECK-NEXT:  %9 = ReturnInst %8: number
// CHECK-NEXT:function_end

// CHECK:function Vec2D(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Math": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "sqrt": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, %2: any, 10: number
// CHECK-NEXT:  %5 = PrStoreInst %4: any, %0: any, 0: number, "x": string, true: boolean
// CHECK-NEXT:  %6 = PrStoreInst %1: any, %0: any, 1: number, "y": string, true: boolean
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end