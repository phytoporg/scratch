# NOTES FOR NEXT TIME

This project is an attempt to build something that allows us to play with RenderDoc as a starting point to a more ambitious project: a tool which will allow us to click on pixels of rendered screens to annotate textures.

This is going to require understanding RenderDoc's architecture to where we can snip out and/or refactor the right pieces of code to serialize the requisite D3D11 calls and state to get pixel->texture mappings in a remote process.

In its current state, this project spins up a simple spinning cube in D3D. Next we want to evaluate:
1) How to hook into the designated target (SFV in this case)
2) How to hook the right D3D calls and serialize those commands to an external project via RenderDoc's frameworks.

So, where to start? Attach to this thing with RenderDoc and debug a locally-built version of RenderDoc to figure out how things work. Figuring out what it would first take to get the end-to-end experience in place would help settle the largest technical risks.
