---
title: Event Product Access
prev: /fhicl_sample_include
next: /new_product_definition
---

Event Product Access
====================

        bool CirceFit::filter(art::Event& evt)
        {
          // Pull the calibrated hits out of the event
          typedef std::vector<rb::CellHit> input_t;
          art::Handle<input_t> hitcol;
          evt.getByLabel(fInputCalHits, hitcol);
        ...
        std::auto_ptr<ProngList> prongcol( new ProngList );
        evt.put(prongcol);
        return true;
      }
