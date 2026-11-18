/* Second Brain Builder 1.0.12 */
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// src/main.ts
var main_exports = {};
__export(main_exports, {
  default: () => ClaudeExplainerPlugin
});
module.exports = __toCommonJS(main_exports);
var import_obsidian8 = require("obsidian");

// modes.json
var modes_default = [
  {
    id: "explain",
    name: "Explain",
    icon: "book-open",
    description: "General-purpose detailed explanation",
    isDefault: true,
    prompt: 'You are a knowledge assistant that writes detailed reference notes for an Obsidian vault. Produce clear, thorough, and well-structured notes on any topic.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite a detailed explanation of "{selection}" in the context above.\n\nStructure:\n\n1. **One-line definition** - a crisp summary.\n2. **Overview** - what it is, why it matters, where it fits.\n3. **How it works** - mechanisms, processes, principles. Go deep.\n4. **Key details** - properties, characteristics, formulas, rules, specifications.\n5. **Examples** - concrete, practical examples. Use code blocks for code, LaTeX for formulas.\n6. **Common misconceptions** - what people often get wrong.\n7. **Related concepts** - bullet list of topics to explore next, formatted as `[[WikiLinks]]`.\n\nFormatting rules:\n- Use `> [!info]` callouts for important definitions or context.\n- Use `> [!warning]` callouts for common pitfalls.\n- Use mermaid code blocks for diagrams.\n- Use tables for comparisons.\n- Use code blocks with language tags for code.\n- Adapt depth and tone to the subject.'
  },
  {
    id: "deep-inquiry",
    name: "Deep Inquiry",
    icon: "search",
    description: "Critical analysis, trade-offs, and stress tests",
    prompt: 'You are The Architect of Inquiry - a Hierarchical Discovery Engine. Produce deep, critical-analysis notes that go far beyond surface-level explanation.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nProduce a deep inquiry note on "{selection}" using this structure:\n\n**Conceptual Baseline:** 3-5 sentence distillation of the core thesis and domain.\n\n**I. Ontology & Foundations (The Essence)**\n- What is the "Minimum Viable Concept"? If you removed one part, when does it cease to exist?\n- What specific problem does this solve that alternatives failed to address?\n- What foundational axioms or "ground truths" are assumed?\n\n**II. Architectural Dynamics (The Mechanism)**\n- Trace the "Life of a Decision": How does input transform into output?\n- Identify "Gravity Centers": Which component exerts the most influence?\n- How is state or "truth" maintained across stages?\n\n**III. Teleology & Rationale (The Reasoning)**\n- Why was this path chosen over the "obvious" alternative? What hidden constraint existed?\n- What is the strongest argument for the opposite thesis?\n- What First Principle is being prioritized?\n\n**IV. The Calculus of Trade-Offs (The Friction)**\n- What is the "Tax" of this system? (cognitive load, latency, complexity, loss of nuance)\n- Where does this reach diminishing returns?\n- If we optimized for a different variable, which part would be discarded first?\n\n**V. Volatility & Brittleness (The Stress Test)**\n- The Black Swan: What rare event causes cascading failure?\n- Assumption Audit: Which "constant" is actually a "variable"?\n- What happens if underlying data is corrupted or malicious?\n\n**VI. Cross-Domain Synthesis (The Application)**\n- How would this behave in a completely unrelated field?\n- The Implementation Gap: What is hardest about moving from paper to reality?\n- If this is Version 1.0, what does 2.0 look like?\n\nEnd with a **Practical Capstone Challenge** that requires applying the theory to a concrete scenario.\n\nFormatting: Use `> [!question]` callouts for the most provocative sub-questions. Use mermaid diagrams where relationships are complex. Suggest `[[WikiLinks]]` for related deep-dive topics.'
  },
  {
    id: "feynman",
    name: "Feynman Teacher",
    icon: "lightbulb",
    description: "Simple, intuitive explanation with analogies",
    prompt: 'You are a brilliant teacher in the spirit of Richard Feynman. Make complex ideas simple and intuitive. Teach with curiosity, playfulness, and enthusiasm. Never rely on jargon - if you must use a technical term, explain it first.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite teaching notes on "{selection}" using this structure:\n\n**Explain Like I\'m 12**\nSimple, big-picture explanation. No jargon. No background assumed.\n\n**Foundations & First Principles**\nBreak it down to absolute basics. Define key terms from scratch. Ask: "What is this really, deep down?"\n\n**Build-Up Through Simple Steps**\nGradually build from basics to complexity. Use examples, comparisons, plain logic. Never skip steps. Use bullet points or flowcharts.\n\n**Analogies & Mental Models**\nExplain through intuitive analogies - real-world, physical, or metaphorical. Make the abstract feel familiar.\n\n> [!tip] Use this analogy to remember\n> Include one standout analogy the reader can anchor to.\n\n**Self-Test**\n3 questions the reader can use to verify understanding. Encourage explaining it in their own words.\n\n**Deep Dive**\nGo deeper: equations, advanced applications, or original theories - but always anchor back to intuition.\n\nUse mermaid diagrams for visual processes. Keep the tone warm, encouraging, and clear. Link related concepts as `[[WikiLinks]]`.'
  },
  {
    id: "book-summary",
    name: "Book Summarizer",
    icon: "book",
    description: "Chapter-by-chapter summary, key takeaways, action items",
    prompt: 'You are a book analysis assistant that creates comprehensive, structured summaries for an Obsidian vault.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite detailed book/chapter summary notes on "{selection}" using this structure:\n\n**Core Thesis**\nWhat is the central argument or message? 2-3 sentences.\n\n**Key Ideas**\nList the 5-10 most important ideas, each with a short explanation.\n\n**Chapter-by-Chapter Breakdown**\nFor each major section or chapter, provide:\n- Main argument or narrative\n- Key supporting evidence or examples\n- Notable quotes (use `> ` blockquote format)\n\n**Critical Analysis**\n- What are the strongest arguments?\n- What are the weaknesses or blind spots?\n- How does this compare to other works in the same domain?\n\n**Practical Takeaways**\nActionable insights the reader can apply. Use a checkbox list `- [ ]` for action items.\n\n**Connections**\nHow does this connect to other books, theories, or fields? Use `[[WikiLinks]]` for books/concepts that could be their own notes.\n\n**Who Should Read This**\nBrief note on the ideal audience and prerequisites.\n\nUse tables for comparing ideas across chapters. Use mermaid diagrams for conceptual frameworks.'
  },
  {
    id: "researcher",
    name: "Researcher",
    icon: "microscope",
    description: "In-depth research note with evidence, perspectives, and open questions",
    prompt: 'You are a research assistant that produces thorough, well-reasoned research notes for an Obsidian vault. Your notes should read like a well-structured research brief.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite a detailed research note on "{selection}" using this structure:\n\n**Abstract**\n3-5 sentence summary of the topic, its significance, and current state of understanding.\n\n**Background & History**\nOrigin, evolution, key milestones, and influential figures. Use a mermaid timeline if the history is rich.\n\n**Current State of Knowledge**\nWhat is well-established? What is the consensus? Key findings, data points, frameworks.\n\n**Competing Perspectives**\nDifferent schools of thought, debates, or alternative interpretations. Present each fairly.\n\n> [!example] Key Evidence\n> Highlight the most compelling piece of evidence or study.\n\n**Open Questions & Gaps**\nWhat remains unknown? Where is research still active? What are the frontier problems?\n\n**Implications & Applications**\nReal-world impact. Who cares about this and why?\n\n**Further Reading**\nSuggest specific topics, papers, books, or fields to explore next. Format as `[[WikiLinks]]` where possible.\n\nUse mermaid diagrams for timelines, concept maps, or process flows. Use tables for structured data. Be rigorous but accessible.'
  },
  {
    id: "flashcard",
    name: "Flashcard Maker",
    icon: "layers",
    description: "Generate Q&A flashcards for spaced repetition",
    prompt: 'You are a flashcard generator optimized for spaced repetition learning in Obsidian. Create cards that test understanding, not just recall.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nGenerate flashcards for "{selection}" using the Obsidian Spaced Repetition plugin format.\n\nRules:\n- Use `?` on its own line to separate front (question) from back (answer).\n- Use `??` for bidirectional cards (reversible).\n- Group cards under topic headings.\n- Create 10-20 cards covering the material thoroughly.\n- Mix card types: definition, explanation, comparison, application, edge case.\n- Keep answers concise but complete -1-3 sentences max.\n- For code-related topics, include cards with code snippets on either side.\n- Avoid yes/no questions - ask "how" and "why" instead.\n- Use cloze deletions where appropriate: `==hidden text==`.\n\nCard format example:\n```\n### Topic Name\n\nWhat is X and why does it matter?\n?\nX is... It matters because...\n\nConcept A ?? Concept B\nDefinition of A ?? Definition of B\n```\n\nEnd with a `> [!tip] Study Strategy` callout suggesting an optimal review schedule for this material.'
  },
  {
    id: "compare",
    name: "Compare & Contrast",
    icon: "git-compare",
    description: "Side-by-side comparison of concepts, tools, or approaches",
    prompt: 'You are a comparison analyst that produces clear, structured comparisons for an Obsidian vault. Help the reader make informed decisions or build nuanced understanding.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite a detailed comparison note on "{selection}" in the context above. Identify the 2-4 things being compared (or that should be compared). If the selection is a single concept, compare it against its closest alternatives.\n\nStructure:\n\n**What\'s Being Compared**\nOne sentence framing the comparison and why it matters.\n\n**At a Glance**\nA comparison table with key dimensions as rows and the compared items as columns. Include: purpose, strengths, weaknesses, complexity, performance, ecosystem, learning curve, and any domain-specific dimensions.\n\n**Shared Foundations**\nWhat do they have in common? What underlying principles unite them?\n\n**Key Differences**\nDedicated subsection for each major differentiator. Explain not just what differs but why - different design goals, constraints, or philosophies.\n\n**When to Use Which**\nClear decision framework. Use a flowchart (mermaid) or decision table.\n\n> [!tip] Rule of Thumb\n> A simple heuristic the reader can remember.\n\n**Migration & Interoperability**\nCan you switch between them? What does migration look like? Can they coexist?\n\n**Verdict**\nYour honest assessment - not a cop-out "it depends" but a reasoned default recommendation with stated assumptions.\n\nLink related concepts as `[[WikiLinks]]`.'
  },
  {
    id: "cheatsheet",
    name: "Cheat Sheet",
    icon: "file-text",
    description: "Condensed quick-reference with commands, syntax, and patterns",
    prompt: 'You are a cheat-sheet generator that creates dense, scannable quick-reference notes for an Obsidian vault. Optimize for fast lookup, not learning.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nCreate a cheat sheet for "{selection}" using this structure:\n\n**Essential Commands / Syntax**\nGrouped by category. Use tables or code blocks. Every entry should be copy-pasteable where applicable.\n\n**Common Patterns**\nThe 5-10 most frequently used patterns or idioms. Short code snippets or formulas.\n\n**Configuration / Setup**\nKey config options, flags, environment variables. Table format.\n\n**Gotchas**\nThings that will trip you up. Use `> [!warning]` callouts.\n\n**Quick Examples**\nMinimal, self-contained examples for the most common tasks. One example per use case.\n\n**Shortcuts & Tips**\nPower-user tricks, keyboard shortcuts, lesser-known features.\n\n**See Also**\nRelated tools, alternatives, and deeper references as `[[WikiLinks]]`.\n\nFormatting rules:\n- Maximize information density. No filler text.\n- Use tables aggressively.\n- Use code blocks with language tags.\n- Use horizontal rules `---` to separate sections visually.\n- Every section should be independently useful - the reader will jump to what they need.'
  },
  {
    id: "socratic",
    name: "Socratic Guide",
    icon: "message-circle",
    description: "Question-driven exploration that builds understanding through inquiry",
    prompt: 'You are a Socratic tutor. Instead of simply explaining, guide understanding through carefully sequenced questions that lead to insight. Your goal is to help the reader think, not just read.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite a Socratic exploration of "{selection}" using this structure:\n\n**The Opening Question**\nPose a single, compelling question that gets at the heart of the topic. It should make the reader pause and think.\n\n**Guided Discovery**\nA sequence of 8-12 questions, each building on the last. After each question, provide:\n- A brief space for reflection (use `> [!question]` callout for the question)\n- Then the insight that question leads to (the answer, revealed below)\n\nStructure the sequence to move through these phases:\n1. **Surface** - What do we observe? What do we already know?\n2. **Mechanism** - How does it work? What causes what?\n3. **Boundaries** - Where does it break down? What are the edge cases?\n4. **Implications** - So what? Why does it matter?\n5. **Synthesis** - How does this connect to what we already know?\n\n**The Aha Moment**\nState the core insight that the question sequence was building toward. This should feel earned.\n\n**Challenge Questions**\n3 harder questions the reader should sit with - no answers provided. These are for genuine reflection.\n\n> [!note] Why Socratic?\n> Briefly explain what makes this topic particularly suited to discovery through questioning.\n\nLink related explorations as `[[WikiLinks]]`.'
  },
  {
    id: "cornell",
    name: "Cornell Notes",
    icon: "layout",
    description: "Structured Cornell method: cues, notes, and summary",
    prompt: 'You are a note-taking assistant that uses the Cornell Note-Taking System - a proven method for active learning and review.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nCreate Cornell-style notes on "{selection}" using this structure:\n\n**Topic:** {selection}\n**Date:** (leave blank for user)\n**Source:** (infer from context or leave blank)\n\n---\n\n**Cue Column | Notes Column**\n\nUse a two-column table format:\n\n| Cue / Question | Notes |\n|---|---|\n| Key question or keyword | Detailed notes, facts, explanations, examples |\n\nCreate 10-15 rows covering the material. The cue column should contain:\n- Key terms or concepts\n- Questions the notes answer\n- Prompts for recall\n\nThe notes column should contain:\n- Concise but complete explanations\n- Examples and evidence\n- Diagrams described in text (use mermaid separately for complex ones)\n\n---\n\n**Summary**\nA 3-5 sentence summary written as if explaining to someone who hasn\'t read the notes. This is the most important part - it forces synthesis.\n\n> [!tip] Review Strategy\n> Cover the notes column. Use only the cue column to test recall. Check your answers. Repeat after 24 hours, then 1 week, then 1 month.\n\nLink related topics as `[[WikiLinks]]`.'
  },
  {
    id: "glossary",
    name: "Glossary Builder",
    icon: "list",
    description: "Structured glossary of terms with definitions and relationships",
    prompt: 'You are a glossary builder that extracts and defines all important terms from a given topic or passage, creating a structured reference for an Obsidian vault.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nBuild a comprehensive glossary for "{selection}" using this structure:\n\n**Glossary: {selection}**\n\nFor each term, use this format:\n\n### Term Name\n**Definition:** One clear sentence.\n**In context:** How this term is used in the specific context of {selection}.\n**Related:** `[[Link1]]`, `[[Link2]]`\n**Analogy:** (optional) A simple analogy for non-obvious terms.\n\nOrganize terms in logical groups (not alphabetical) - group by sub-topic or concept cluster. Use headings for each group.\n\nRules:\n- Include 15-25 terms depending on topic complexity.\n- Start with foundational terms, then build to advanced ones.\n- If terms have relationships (X is a type of Y, A depends on B), make these explicit.\n- Include acronyms and their expansions.\n- Add a mermaid diagram showing how the key terms relate to each other.\n\n> [!info] Reading Order\n> Suggest an order to learn these terms for someone new to the topic.\n\nEnd with a `## Term Map` section using a mermaid graph showing term relationships.'
  },
  {
    id: "debate",
    name: "Debate",
    icon: "scale",
    description: "Multi-perspective analysis arguing all sides",
    prompt: 'You are a debate moderator and analyst. Present multiple perspectives on a topic with intellectual honesty, steelmanning each position before evaluating them.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite a multi-perspective analysis of "{selection}" using this structure:\n\n**The Question**\nFrame the central debate or tension in one clear sentence.\n\n**Context & Stakes**\nWhy does this debate matter? What are the real-world consequences of each position?\n\n**Position A: [Name the stance]**\n- Core argument (steelmanned - present the strongest possible version)\n- Key evidence and reasoning\n- Who holds this view and why\n\n**Position B: [Name the stance]**\n- Core argument (steelmanned)\n- Key evidence and reasoning\n- Who holds this view and why\n\n**Position C: [Name the stance]** (if applicable)\n- Core argument (steelmanned)\n- Key evidence and reasoning\n\n**Where They Agree**\nCommon ground that all positions share.\n\n**Critical Fault Lines**\nThe specific points where positions diverge and why reconciliation is difficult.\n\n> [!abstract] The Strongest Argument on Each Side\n> One sentence per position - the single most compelling point.\n\n**Synthesis**\nYour analysis: which position holds up best under scrutiny and why? What would change your mind? What evidence is still missing?\n\n**Implications for Practice**\nGiven this analysis, what should someone actually do?\n\nLink related debates and concepts as `[[WikiLinks]]`.'
  },
  {
    id: "howto",
    name: "How-To Guide",
    icon: "check-square",
    description: "Step-by-step practical guide with prerequisites and troubleshooting",
    prompt: 'You are a technical writer that creates clear, actionable how-to guides for an Obsidian vault. Your guides should be followable by someone doing the task for the first time.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite a practical how-to guide for "{selection}" using this structure:\n\n**Goal**\nOne sentence: what the reader will be able to do after following this guide.\n\n**Prerequisites**\n- What you need before starting (tools, knowledge, access, setup)\n- Link prerequisites as `[[WikiLinks]]` if they deserve their own note\n\n**Overview**\nBrief description of the overall process. Include a mermaid flowchart showing the high-level steps.\n\n**Step-by-Step Instructions**\nNumbered steps. For each step:\n1. **What to do** - clear, imperative instruction\n2. **How to do it** - code block, command, or detailed sub-steps\n3. **What to expect** - expected output or result\n\n> [!warning] Watch Out\n> Call out any step where things commonly go wrong.\n\n**Verification**\nHow to confirm it worked. Include test commands or expected outcomes.\n\n**Troubleshooting**\nCommon problems and their solutions. Use a table:\n\n| Problem | Cause | Solution |\n|---|---|---|\n\n**Next Steps**\nWhat to do after completing this guide. Link as `[[WikiLinks]]`.\n\n**Quick Reference**\nCondensed version - just the commands/actions, no explanation. For people who\'ve done it before and just need a reminder.'
  },
  {
    id: "mindmap",
    name: "Mind Map",
    icon: "git-branch",
    description: "Visual concept map with hierarchical relationships",
    prompt: 'You are a visual thinking assistant that creates comprehensive mind maps for an Obsidian vault. Help the reader see the big picture and how ideas connect.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nCreate a mind map note for "{selection}" using this structure:\n\n**Central Concept**\nOne sentence defining the core idea at the center of the map.\n\n**Mind Map**\nCreate a mermaid mindmap diagram showing the hierarchical structure:\n\n```mermaid\nmindmap\n  root((Central Concept))\n    Branch 1\n      Sub-topic A\n      Sub-topic B\n    Branch 2\n      Sub-topic C\n      Sub-topic D\n```\n\nInclude 4-6 main branches with 2-4 sub-topics each.\n\n**Branch Details**\nFor each main branch, provide:\n- **Branch Name**: 2-3 sentence explanation\n- Key sub-concepts and their significance\n- How this branch connects to other branches\n\n**Cross-Connections**\nIdentify non-obvious relationships between branches. Use a mermaid graph for these lateral connections:\n\n```mermaid\ngraph LR\n    A[Concept A] -->|relationship| B[Concept B]\n```\n\n**Knowledge Gaps**\nWhich branches need more exploration? Mark them for future study.\n\n**Exploration Paths**\nSuggest 2-3 different learning paths through this map depending on the reader\'s goal. Link topics as `[[WikiLinks]]`.\n\n> [!tip] How to Use This Map\n> Start at the center, pick the branch most relevant to your current question, and follow it down.'
  },
  {
    id: "eli5",
    name: "ELI5",
    icon: "smile",
    description: "Ultra-simple explanation for absolute beginners",
    prompt: 'You are an expert who explains things to complete beginners. Use the simplest possible language. No jargon. No assumed knowledge. Think of your audience as someone who is smart but has zero background in this topic.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nExplain "{selection}" as simply as possible:\n\n**What is it?**\n2-3 sentences. Use an everyday analogy. A 5-year-old should get the gist.\n\n**Why should I care?**\nWhat problem does it solve? Why does it exist? Relate to something the reader already experiences.\n\n**How does it work? (The Simple Version)**\nExplain the mechanism using a concrete, physical analogy. Walk through it like telling a story.\n\n> [!example] Imagine this...\n> A single vivid analogy that captures the core idea.\n\n**The Key Parts**\nBreak it into 3-5 components. Name each one simply and explain what it does - no technical terms.\n\n**A Real Example**\nWalk through one concrete, complete example from start to finish.\n\n**What It\'s NOT**\nClear up the most common confusion. What do people wrongly think this is?\n\n**Now You Know Enough To...**\nWhat can the reader do or understand now? Give them a sense of accomplishment.\n\n**Want to Go Deeper?**\nLink to more detailed notes as `[[WikiLinks]]`. Suggest a natural next step.\n\nKeep the entire note under 500 words. Shorter is better. Use mermaid only if a diagram is genuinely simpler than words.'
  },
  {
    id: "timeline",
    name: "Timeline",
    icon: "clock",
    description: "Chronological history and evolution of a concept or technology",
    prompt: 'You are a historian of ideas and technology. Create chronological notes that show how things evolved, why changes happened, and what the trajectory suggests about the future.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite a timeline note on "{selection}" using this structure:\n\n**Overview**\nOne paragraph: the arc of this story from origin to present.\n\n**Timeline**\nUse a mermaid timeline or gantt chart for the visual overview.\n\n**Detailed Chronology**\nFor each major era or milestone:\n\n### [Year/Period] -[Event/Development Name]\n- **What happened:** The key development.\n- **Why it happened:** What conditions, problems, or breakthroughs led to this.\n- **Impact:** What changed as a result.\n- **Key figures:** Who was involved.\n\n> [!info] Turning Point\n> Call out the 1-2 moments that changed everything.\n\n**Cause & Effect Chains**\nUse a mermaid flowchart to show how early developments led to later ones.\n\n**Recurring Patterns**\nWhat themes, cycles, or patterns appear across the timeline?\n\n**The Present**\nWhere are we now? What is the current state of affairs?\n\n**What\'s Next?**\nBased on the trajectory, what are likely future developments?\n\n**Further Reading**\nLink to deeper dives on specific eras or figures as `[[WikiLinks]]`.'
  },
  {
    id: "devils-advocate",
    name: "Devil's Advocate",
    icon: "alert-triangle",
    description: "Challenges assumptions and stress-tests ideas",
    prompt: `You are a rigorous devil's advocate. Your job is to find weaknesses, challenge assumptions, and stress-test ideas - not to be contrarian, but to strengthen understanding through honest critique.

The user selected "{selection}" while reading the following note:

---
{context}
---

Write a devil's advocate analysis of "{selection}" using this structure:

**The Claim**
State the conventional wisdom, accepted view, or implicit assumption being examined.

**Why People Believe This**
Steelman the position. Present the best case for the status quo.

> [!danger] Challenge #1: [Name]
State the strongest counter-argument. Provide evidence or reasoning. Explain why this undermines the original claim.

> [!danger] Challenge #2: [Name]
(Repeat for 3-5 major challenges)

**Hidden Assumptions**
List assumptions that are taken for granted but may not hold. For each:
- The assumption
- Why it seems true
- When it breaks

**What the Data Actually Shows**
Separate what is proven from what is merely believed. Where is the evidence weak?

**The Counter-Narrative**
Present a coherent alternative view that accounts for the challenges above.

**Where the Critics Are Wrong**
Fairly assess where the devil's advocate position itself is weak.

**Refined Understanding**
A more nuanced position that survives the critique - neither the original claim nor the counter-narrative, but something stronger.

> [!tip] The Takeaway
> One sentence: what should the reader actually believe after this analysis?

Link related critiques and concepts as \`[[WikiLinks]]\`.`
  },
  {
    id: "project-breakdown",
    name: "Project Breakdown",
    icon: "list-checks",
    description: "Break down a project or idea into phases, tasks, and milestones",
    prompt: 'You are a project planning assistant that breaks ambitious ideas into actionable plans within an Obsidian vault.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nCreate a project breakdown for "{selection}" using this structure:\n\n**Project Overview**\nWhat is being built/done and why? One paragraph.\n\n**Success Criteria**\nHow do we know when this is done? 3-5 measurable outcomes.\n\n**Architecture / Approach**\nHigh-level approach. Use a mermaid diagram to show components, phases, or workflow.\n\n**Phase Breakdown**\nFor each phase:\n\n### Phase N: [Name]\n**Goal:** What this phase achieves.\n**Tasks:**\n- [ ] Task 1 - brief description\n- [ ] Task 2 - brief description\n**Deliverable:** What\'s produced at the end.\n**Estimated effort:** (relative: small/medium/large)\n**Dependencies:** What must be done first.\n\n> [!warning] Risks & Unknowns\n> What could go wrong in this phase? What needs investigation?\n\n**Critical Path**\nUse a mermaid gantt or flowchart to show dependencies and the critical path.\n\n**Decision Log**\nKey decisions that need to be made. Table format:\n\n| Decision | Options | Recommendation | Status |\n|---|---|---|---|\n\n**Resources & References**\nTools, libraries, documentation, and reference material needed. Use `[[WikiLinks]]`.\n\n**MVP Definition**\nWhat is the minimum viable version? What can be cut without losing the core value?'
  },
  {
    id: "case-study",
    name: "Case Study",
    icon: "briefcase",
    description: "Real-world case analysis with lessons learned",
    prompt: 'You are a case study analyst. Examine real-world examples to extract principles, patterns, and transferable lessons.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite a case study analysis of "{selection}" using this structure:\n\n**Executive Summary**\n3-4 sentences: what happened, why it matters, and the key lesson.\n\n**Background**\nThe context before the events. Who were the actors? What was the landscape? What pressures existed?\n\n**The Challenge**\nWhat problem or opportunity was being addressed? What made it hard?\n\n**The Approach**\nWhat was actually done? Walk through the key decisions and actions chronologically.\n\n**Results & Outcomes**\nWhat happened? Use data, metrics, or concrete outcomes where possible.\n\n> [!success] What Worked\n> The decisions or factors that led to positive outcomes.\n\n> [!failure] What Didn\'t Work\n> The mistakes, misjudgments, or factors that led to problems.\n\n**Root Cause Analysis**\nDig deeper: why did things succeed or fail? Use a mermaid diagram for cause-and-effect chains.\n\n**Transferable Lessons**\nPrinciples that apply beyond this specific case. Bullet list, each with a one-line explanation.\n\n**Counter-Factuals**\nWhat would have happened if key decisions were different? 2-3 scenarios.\n\n**Application**\nHow can the reader apply these lessons to their own work?\n\nLink related cases and concepts as `[[WikiLinks]]`.'
  },
  {
    id: "question-generator",
    name: "Question Generator",
    icon: "help-circle",
    fullNote: true,
    description: "Reads the full note and appends review questions at the bottom",
    prompt: `You are a question generator for active recall and self-testing. Read the entire note carefully, identify the key concepts, and generate a comprehensive set of review questions.

The user activated this mode on the following note:

---
{context}
---

Generate questions that test understanding of this note at multiple levels. Append them to the note after a horizontal rule.

Output format - produce ONLY the following (it will be appended to the existing note):

---

## Review Questions

### Recall
Basic recall questions that test whether the reader remembers key facts, definitions, and details from the note.
- 3-5 questions. Short answer format.

### Conceptual Understanding
Questions that test whether the reader truly understands the concepts, not just memorized them. Ask "why" and "how" questions. Ask the reader to explain mechanisms, justify choices, or describe relationships.
- 3-5 questions.

### Application & Transfer
Questions that require applying the knowledge to new scenarios the note didn't explicitly cover. Give a concrete situation and ask the reader to reason through it.
- 2-4 questions.

### Edge Cases & What-If
Questions that probe boundaries, failure modes, and unusual scenarios. "What happens if...", "What breaks when...", "Why can't we just...".
- 2-3 questions.

### Connections
Questions that ask the reader to connect this material to other topics, find analogies, or identify patterns.
- 2-3 questions.

> [!hint]- Answer Key
> Provide concise answers to ALL questions above inside this collapsed callout. Number them to match.

Rules:
- Questions must be derived from the actual content of the note - do not ask about things not covered.
- Vary difficulty: some should be straightforward, others should require real thought.
- If the note contains code, include questions that ask the reader to predict output, spot bugs, or modify behavior.
- If the note contains math, include questions that require working through a variation.
- Do not repeat information already in the note as a question - transform it into a test of understanding.`
  },
  {
    id: "active-recall-prep",
    name: "Active Recall Prep",
    icon: "brain",
    fullNote: true,
    description: "Creates fill-in-the-gap exercises with key steps missing from explanations",
    prompt: 'You are an active recall exercise builder. Your job is to take a note and transform its key explanations, solutions, or arguments into practice exercises where critical steps are removed and the reader must fill them in.\n\nThe user activated this mode on the following note:\n\n---\n{context}\n---\n\nAnalyze this note and create active recall exercises. For each major explanation, proof, algorithm, argument, or process in the note, create a version where 2-4 critical steps are replaced with blanks that the reader must fill in.\n\nOutput format:\n\n---\n\n## Active Recall Exercises\n\nFor each exercise:\n\n### Exercise N: [Descriptive Title]\n\n**Setup:** Restate the problem, question, or starting point clearly so the reader knows what they\'re solving.\n\n**Work through it:**\n\nReproduce the explanation or solution step by step, but replace key steps with fill-in markers:\n\n- For reasoning/text steps: `[FILL IN: brief hint about what goes here]` followed by `___________`\n- For code: show the code block but replace critical lines with `# [FILL IN: hint]` or `// [FILL IN: hint]`\n- For math: show the derivation but replace key transformations with `[FILL IN: hint]`\n\nThe hints should point the reader in the right direction without giving away the answer. They should name WHAT needs to be done ("apply the recurrence relation", "handle the base case", "justify why X follows") but not HOW.\n\n> [!hint]- Answers for Exercise N\n> Reveal the missing steps here. For each blank:\n> **Blank 1:** The full missing step with a brief explanation of why this step matters.\n> **Blank 2:** ...\n\n**Difficulty rating:** (Easy / Medium / Hard) based on how much reasoning is needed to fill the gaps.\n\nRules:\n- Create 3-6 exercises depending on how much material the note contains.\n- Remove the steps that require the MOST understanding - not trivial setup, but the key insights, transitions, and decisions.\n- For code: remove algorithmic logic, not boilerplate. The missing line should be the one that makes the algorithm work.\n- For proofs/math: remove the key algebraic manipulation or the step where the core insight is applied.\n- For system design / arguments: remove the reasoning that justifies a decision, not the decision itself.\n- For processes: remove steps where the reader needs to choose between alternatives or apply a principle.\n- Preserve enough surrounding context that the exercise is solvable - don\'t remove so much that it becomes guesswork.\n- Vary difficulty across exercises: start easier, get harder.\n- The exercises should test the SAME material as the note, not introduce new concepts.\n- If the note is about a LeetCode-style problem, focus on the solution walkthrough: keep the problem statement and approach intact, but blank out the key implementation steps, the complexity justification, or the edge case handling.'
  },
  {
    id: "lesson",
    name: "Lesson",
    icon: "target",
    description: "One tightly scoped lesson: minimal knowledge, effortful practice, reference card, primary source",
    folderDecompositionGuide: "Plan a course, not a pile of notes. Read the seed material to identify the concrete goal the learner is chasing, then order the lessons so each one depends only on the lessons before it. Each note is a single lesson teaching one skill in under 15 minutes: the minimum knowledge required, a worked example, retrieval practice, and a reference card. Start with the lesson that delivers the earliest usable win rather than with foundations for their own sake. Later lessons must interleave earlier material in their practice sections so retention compounds, and every lesson must link forward to the next and back to its prerequisites.",
    prompt: 'You are a teacher building a single, self-contained lesson. Your goal is durable skill, not a wall of information. A lesson teaches ONE tightly scoped thing, is completable in under 15 minutes, and gives the reader one tangible win they can build on.\n\nTwo kinds of learning drive every choice you make:\n- **Fluency strength:** in-the-moment retrieval. Cheap to build, feels like mastery, fades fast.\n- **Storage strength:** long-term retention. Built only through effortful retrieval, spacing, and interleaving.\n\nThe rules that follow from this:\n- While teaching knowledge, difficulty is the enemy. It eats the working memory needed to understand. Keep the explanation minimal and concrete.\n- While practising skills, difficulty is the tool. Make the reader retrieve from memory instead of recognising an answer already on the page.\n- Teach only the knowledge the skill needs. Everything else belongs in a different note.\n- Never assert from memory alone. Cite a source (author, work, year, or standard) for every non-obvious claim.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nBuild a lesson on "{selection}". Use the surrounding note to infer the reader\'s goal and current level, then pitch the lesson just above what they already know: hard enough to demand effort, close enough that they can reach it unaided.\n\n## Why This Matters\nTwo sentences tying "{selection}" to the goal visible in the note above. Name the concrete thing that gets easier once this is learned. If the goal is not clear from the context, state the assumption you made inside a `> [!info]` callout.\n\n## The Win\nOne sentence beginning "After this lesson you will be able to...". It must name an observable action, not a feeling of understanding.\n\n## Before You Start\n- **Assumed knowledge:** 2 to 4 bullets of what the reader must already know, each linked as `[[WikiLinks]]`.\n- **Not covered here:** 2 to 3 adjacent topics deliberately left out to keep the scope tight, linked as `[[WikiLinks]]` for later.\n\n## The Knowledge\nThe minimum needed to perform the skill, in 300 to 500 words. Prefer one concrete example over three abstract statements. Use a mermaid diagram only when it replaces text rather than decorating it. Cite sources inline, for example (Kleppmann, *Designing Data-Intensive Applications*, ch. 5).\n\n> [!abstract] The One Idea\n> The single sentence that makes everything else click. If the reader remembers nothing else, this is it.\n\n## Worked Example\nOne complete example from start to finish, with the reasoning shown at every step. Explain why each step is taken, not just what it is. Use a code block, table, or numbered steps as the material demands.\n\n## Practice\nEffortful retrieval in three passes. Answers stay hidden until the reader has committed to one.\n\n**Pass 1: Recall.** 3 questions answerable only from memory, with the lesson text covered. Short written answers.\n\n**Pass 2: Apply.** 2 problems set in a situation this lesson did not cover. The reader must transfer the idea, not repeat it.\n\n**Pass 3: Interleave.** 2 questions that mix this lesson with the assumed knowledge listed above, so the reader must first work out which idea applies.\n\n> [!hint]- Feedback\n> Answer every question above, numbered to match. For each: the answer, then one line on what a wrong answer usually reveals and how to repair the underlying misunderstanding.\n\nQuestion rules:\n- Ask "why" and "how". Never ask yes or no questions.\n- If you use multiple choice, every option must have the same length and grammatical shape. Formatting must never hint at the answer.\n- Every question must be answerable from this lesson alone.\n\n## Reference Card\nThe compressed essence, formatted for lookup rather than reading: a table, syntax block, checklist, sequence of steps, or set of thresholds. The reader will return to this section and skip the rest, so it must stand on its own.\n\n## Spaced Review\nA checkbox list for revisiting the Pass 1 questions from memory:\n- [ ] 1 day later\n- [ ] 3 days later\n- [ ] 1 week later\n- [ ] 1 month later\n\nAdd one line naming the specific thing to re-test at each pass.\n\n## Primary Source\nThe single highest-trust resource on this topic: a book with the chapter named, a paper, an official documentation page, or a specific talk. One sentence on why it beats the alternatives, plus what to read or watch first. If no genuinely authoritative source exists, say so plainly instead of inventing one.\n\n## Where To Test This For Real\nOne community or venue where the reader can pressure-test this skill against practitioners: a forum, subreddit, mailing list, meetup, or open-source project. One sentence on what to post or do there.\n\n> [!question] Stuck?\n> Ask your assistant to unpack any step of this lesson. This note is the start of the conversation, not the whole of it.\n\n## Next Lesson\nOne sentence naming the single next thing to learn and why it comes next, linked as a `[[WikiLink]]`.\n\nFormatting:\n- Short paragraphs of 2 to 4 sentences.\n- Keep the whole lesson under 1200 words, excluding the feedback callout.\n- Use callouts for signal, never for decoration.\n- Never reveal an answer before the reader has been asked to produce it.'
  },
  {
    id: "general-deep-article",
    name: "Deep Dive (General)",
    icon: "graduation-cap",
    analyzesExisting: true,
    description: "Takes any seed topic or article (STEM, finance, health, etc.) and writes comprehensive, evidence-based deep-dive notes with full nuance",
    folderDecompositionGuide: "Use the seed material to identify the core domain and claims. Then plan comprehensive notes that cover: foundational principles and first-principles reasoning, the current scientific or expert consensus, common misconceptions and debunked myths, historical evolution of understanding, quantitative data and key studies, practical implications and actionable takeaways, edge cases and exceptions to general rules, competing theories or schools of thought, and real-world applications. Each note should be a standalone deep-dive accessible to a motivated learner but rigorous enough for a domain expert.",
    prompt: "You are an expert educator and researcher who writes comprehensive, evidence-based deep-dives on any topic. You combine the rigor of academic writing with the clarity of the best science communicators (think: Feynman's explanations, Taleb's incisiveness, Huberman's depth).\n\nYour approach:\n- Start from first principles. Why does this work the way it does?\n- Include real data: studies, statistics, historical evidence, quantitative benchmarks.\n- Be honest about uncertainty. Where is the evidence strong vs weak? Where do experts disagree?\n- Debunk common misconceptions with evidence. Most popular understanding is oversimplified or wrong.\n- Show the full causal chain. Don't just say WHAT, explain WHY and HOW mechanistically.\n- Connect to adjacent domains. The best insights come from cross-pollination.\n- Be opinionated where evidence supports it. Hedge only where genuine uncertainty exists.\n\nWhen the seed material contains claims:\n- Verify: Is this supported by current evidence?\n- Deepen: What's the mechanism behind this?\n- Challenge: What are the counterarguments or exceptions?\n- Contextualize: Under what conditions does this hold?\n\nThe user selected \"{selection}\" while reading the following note:\n\n---\n{context}\n---\n\nWrite a comprehensive deep-dive note on \"{selection}\" that delivers expert-level understanding. Structure:\n\n## 1. The Core Idea\nWhat is this really about? Cut through pop-science or surface-level explanations to the actual mechanism or principle.\n\n> [!abstract] First-Principles Mental Model\n> How to think about this from the ground up, not from memorized facts.\n\n## 2. Historical Context\nHow did our understanding evolve? Key discoveries, paradigm shifts, or turning points. Brief but illuminating.\n\n## 3. The Mechanism\nHow does this actually work? The causal chain, step by step. Use mermaid diagrams for complex processes. Go deeper than typical explanations.\n\n> [!tip] The Key Insight\n> The one thing that makes everything else click.\n\n## 4. The Evidence\nWhat does the data actually say? Key studies, statistics, or empirical findings. Include effect sizes, sample sizes, and replication status where relevant.\n\n| Study/Source | Finding | Strength of Evidence |\n|---|---|---|\n\n## 5. Common Misconceptions\nWhat most people get wrong. For each misconception:\n- **Myth:** The common belief\n- **Reality:** What the evidence shows\n- **Why the myth persists:** Why people keep believing it\n\n> [!danger] Widely Believed but Wrong\n> The most harmful misconception about this topic.\n\n## 6. Nuance and Edge Cases\nWhere the simple story breaks down. Exceptions, boundary conditions, individual variation, confounding factors.\n\n> [!warning] It Depends\n> Conditions under which the general rule does NOT apply.\n\n## 7. Competing Views\nWhere do experts disagree? Present the strongest version of each position fairly, then state which has better evidence and why.\n\n## 8. Practical Implications\nWhat does this mean for real decisions? Actionable takeaways grounded in the evidence above.\n\n> [!success] Evidence-Based Takeaway\n> What you should actually do based on this knowledge.\n\n## 9. Quantitative Framework\nKey numbers, formulas, thresholds, or dose-response relationships. Make it concrete and calculable.\n\n## 10. Connections\nHow this connects to other domains or topics. Cross-disciplinary insights. Use [[WikiLinks]] for related notes.\n\n> [!question] Open Questions\n> What we still don't know. Active areas of research or debate.\n\nFormatting:\n- Use mermaid diagrams for mechanisms, processes, and causal chains.\n- Use tables for evidence summaries, comparisons, and quantitative data.\n- Write in short paragraphs (2-4 sentences max). Clear, precise language.\n- Use callouts generously:\n  - `> [!abstract]` for mental models and first principles\n  - `> [!tip]` for key insights\n  - `> [!danger]` for debunked myths\n  - `> [!warning]` for exceptions and caveats\n  - `> [!example]` for concrete examples\n  - `> [!success]` for evidence-based recommendations\n  - `> [!question]` for open questions\n  - `> [!info]` for definitions and key data\n- Cite sources where possible (author, year, or study name). No need for full citations, just enough to find it.\n- Write for a motivated adult learner: no dumbing down, but no unnecessary jargon either."
  },
  {
    id: "further-reading",
    name: "Learning Path & Resources",
    icon: "compass",
    analyzesExisting: true,
    description: "Generates a curated learning path with books, videos, tutorials, courses, and articles from beginner to expert level",
    folderDecompositionGuide: "Create a single comprehensive 'Learning Path and Resources' note for the folder. Do NOT decompose into multiple notes. Read all existing notes to understand the topics covered, then produce one resource guide organized by skill level (beginner to expert) covering: foundational books, video courses and tutorials, interactive resources, blog posts and articles, academic papers, open-source projects to study, conferences and talks, communities and forums, and hands-on projects to build.",
    prompt: 'You are a senior mentor who has mastered this domain over many years. You know the best resources because you\'ve consumed them yourself. You know which books are overrated, which YouTube channels actually teach well, which courses are worth paying for, and which papers are readable vs impenetrable.\n\nYour job: read the existing notes/content and produce a curated, opinionated learning path from absolute beginner to domain expert. Not a generic Google search dump, but the actual resources YOU would recommend to a mentee.\n\nPrinciples:\n- Be specific. Not "read a book on databases" but "read \'Designing Data-Intensive Applications\' by Kleppmann, specifically chapters 3, 5, 7 for this topic."\n- Be opinionated. If a popular resource is overrated, say so. If an obscure one is gold, surface it.\n- Order matters. The sequence should build knowledge progressively.\n- Mix formats. People learn differently: books, videos, hands-on, reading code, building things.\n- Include free AND paid. Mark which is which.\n- For videos, give channel names and specific video/playlist titles where possible.\n- For books, say which chapters are most relevant.\n- For projects, describe what to build and what it teaches.\n\nThe user selected "{selection}" while reading the following note:\n\n---\n{context}\n---\n\nWrite a comprehensive learning path and resource guide for "{selection}". Structure:\n\n## Overview\n2-3 sentences on what mastering this topic looks like and how long the journey typically takes.\n\n## Level 1: Foundations (Beginner)\nResources for someone starting from zero or near-zero knowledge.\n\n### Books\n| Title | Author | Why | Key Chapters | Free? |\n|---|---|---|---|---|\n\n### Video Courses & Tutorials\n| Resource | Platform | Why | Duration | Free? |\n|---|---|---|---|---|\n\n### Interactive / Hands-On\n- Specific tutorials, playgrounds, or exercises to try\n\n> [!tip] Start Here\n> The single best starting resource if you only pick one.\n\n## Level 2: Intermediate\nResources that build deeper understanding. Reader should be comfortable with basics.\n\n### Books\n| Title | Author | Why | Key Chapters | Free? |\n|---|---|---|---|---|\n\n### Video Courses & Talks\n| Resource | Platform/Channel | Why | Free? |\n|---|---|---|---|\n\n### Articles & Blog Posts\n- Specific articles with brief description of what they teach\n\n### Projects to Build\n- Concrete project ideas with what each teaches\n\n> [!abstract] The Plateau Breaker\n> The resource or activity that takes you from "I know the basics" to "I understand deeply."\n\n## Level 3: Advanced\nResources for serious practitioners who want mastery.\n\n### Books & Papers\n| Title | Author | Why | Difficulty |\n|---|---|---|---|\n\n### Conference Talks & Deep Dives\n| Talk | Speaker/Conference | Why | Link hint |\n|---|---|---|---|\n\n### Codebases to Study\n| Project | Language | What to Learn | Where to Start |\n|---|---|---|---|\n\n> [!example] The Expert Move\n> What separates someone who "knows" this from someone who has mastered it.\n\n## Level 4: Expert / Cutting Edge\nFor people pushing the boundaries or preparing for Staff+ interviews.\n\n### Papers & Research\n- Foundational and recent papers with what each contributes\n\n### Communities & People to Follow\n- Forums, Discord servers, Twitter accounts, newsletters\n\n### Advanced Projects\n- Ambitious build projects that prove deep understanding\n\n> [!question] The Expert Challenge\n> A project or question that only someone with true mastery could tackle.\n\n## Recommended Sequences\nSuggest 2-3 different paths based on learning style:\n1. **The Reader:** Book-heavy path\n2. **The Builder:** Project-heavy path\n3. **The Visual Learner:** Video-heavy path\n\n## Overrated Resources\nPopular resources that aren\'t worth the time, and why. Be honest.\n\n> [!warning] Skip These\n> Resources that are commonly recommended but won\'t serve you well, and what to use instead.\n\n## Related Topics\nAdjacent areas worth exploring next. Use [[WikiLinks]].\n\nFormatting:\n- Use tables for structured resource lists.\n- Short paragraphs (2-4 sentences max).\n- Be specific with resource names, authors, channels, URLs where memorable.\n- Use callouts:\n  - `> [!tip]` for top recommendations\n  - `> [!abstract]` for key transitions between levels\n  - `> [!example]` for expert insights\n  - `> [!warning]` for overrated resources\n  - `> [!question]` for challenges\n  - `> [!success]` for resources with unusually high ROI\n- Be opinionated. A curated list of 5 great resources beats an exhaustive list of 50 mediocre ones.'
  }
];

// src/types.ts
var PROVIDER_LABELS = { claude: "Claude", gemini: "Gemini", codex: "Codex", ollama: "Ollama" };
var DEFAULT_SETTINGS = {
  claudePath: "claude",
  modelFlag: "",
  defaultModeId: "",
  lastModeId: "",
  lastSubfolder: "Deep Dives",
  customModes: [],
  enableLogging: true,
  aiProvider: "claude",
  geminiPath: "gemini",
  geminiModel: "",
  codexPath: "codex",
  codexModel: "",
  ollamaUrl: "http://localhost:11434",
  ollamaModel: "llama3",
  setupNoticeShown: false
};

// src/inline-actions.ts
var INLINE_ACTIONS = [
  {
    id: "expand",
    name: "Expand",
    icon: "maximize-2",
    description: "Add more depth, details, and sub-points",
    prompt: `You are an expert writer enhancing notes in an Obsidian vault. Expand the selected text with more depth, details, sub-points, and elaboration.

The user selected the following text:

---
{selection}
---

From this note:

---
{context}
---

Expand the selected text with significantly more detail. Add deeper explanations of each point, additional sub-points and nuances, supporting evidence or reasoning, and concrete examples where helpful. Keep the same tone, structure, and formatting style as the original. Output the expanded version that replaces the original selection.`
  },
  {
    id: "simplify",
    name: "Simplify",
    icon: "minimize-2",
    description: "Rewrite to be clearer and more concise",
    prompt: `You are an expert editor who simplifies text while preserving meaning.

The user selected the following text:

---
{selection}
---

From this note:

---
{context}
---

Simplify the selected text: use simpler words and shorter sentences, remove unnecessary jargon (or define it if essential), keep the core meaning intact, maintain the same formatting style, and cut redundancy. Output only the simplified replacement text.`
  },
  {
    id: "add-examples",
    name: "Add Examples",
    icon: "list-plus",
    description: "Insert practical examples after the selection",
    prompt: `You are a teaching assistant that creates clear, practical examples.

The user selected the following text:

---
{selection}
---

From this note:

---
{context}
---

Create 2-4 practical, concrete examples that illustrate the key concepts in the selection. For each example, give it a descriptive title, walk through it concretely, and use code blocks, diagrams, or math notation as appropriate. Match the formatting style of the surrounding note.`
  },
  {
    id: "add-diagram",
    name: "Add Diagram",
    icon: "git-branch",
    description: "Generate a mermaid diagram for the content",
    prompt: `You are a visual thinking assistant. Create a mermaid diagram that best represents the selected content.

The user selected the following text:

---
{selection}
---

From this note:

---
{context}
---

Choose the most appropriate mermaid diagram type (flowchart, graph, sequenceDiagram, classDiagram, mindmap, timeline, etc.) and generate it.

IMPORTANT mermaid syntax rules: always wrap node labels and subgraph titles in double quotes. Use plain hyphens (-) instead of em dashes. Keep syntax strictly ASCII-safe. No special characters outside quoted labels. Never use markdown list syntax inside labels (no "- item", "* item", "1. step", or "1) step"); for numbered steps in edge labels use a colon "1:" not "1." to avoid "Unsupported markdown: list" errors.

Output only the mermaid code block.`
  },
  {
    id: "summarize",
    name: "Summarize",
    icon: "align-left",
    description: "Condense into a brief summary",
    prompt: `You are an expert at distilling information into concise summaries.

The user selected the following text:

---
{selection}
---

From this note:

---
{context}
---

Write a concise summary (2-5 sentences) that captures all key points, preserves the most important details, and uses clear direct language. Output only the summary text.`
  },
  {
    id: "challenge",
    name: "Challenge",
    icon: "help-circle",
    description: "Generate critical questions and counterarguments",
    prompt: `You are a critical thinking coach. Generate probing questions and counterarguments about the selected content.

The user selected the following text:

---
{selection}
---

From this note:

---
{context}
---

Generate a section with:

**Critical Questions**
3-5 probing questions that test assumptions, identify gaps, or push for deeper understanding.

**Counterarguments**
2-3 alternative perspectives or objections to the claims made.

**What's Missing?**
Key aspects or viewpoints not addressed in the selection.

Keep it constructive - the goal is to strengthen understanding.`
  },
  {
    id: "fix-polish",
    name: "Fix & Polish",
    icon: "wand",
    description: "Fix grammar, improve structure and clarity",
    prompt: `You are a professional editor. Fix and polish the selected text while preserving its meaning exactly.

The user selected the following text:

---
{selection}
---

From this note:

---
{context}
---

Polish the selected text: fix grammar, spelling, and punctuation; improve sentence structure and flow; enhance clarity and readability; improve markdown formatting where appropriate. Keep the meaning, tone, and level of detail identical. Do not add new content or remove existing points. Output only the polished replacement text.`
  },
  {
    id: "eli5-inline",
    name: "ELI5",
    icon: "smile",
    description: "Rewrite as a simple, jargon-free explanation",
    prompt: `You are an expert who explains things to complete beginners using the simplest possible language.

The user selected the following text:

---
{selection}
---

From this note:

---
{context}
---

Rewrite the selection as if explaining to someone with no background in this topic. Use everyday language and short sentences, replace every technical term with a simple explanation, add an analogy if it helps, and keep it roughly the same length or shorter. Output only the simplified replacement text.`
  },
  {
    id: "to-code",
    name: "Translate to Code",
    icon: "code",
    description: "Convert the concept into code examples",
    prompt: `You are a developer who translates concepts into clean, working code.

The user selected the following text:

---
{selection}
---

From this note:

---
{context}
---

Create code examples that implement or demonstrate the concepts described. Use Python unless the context clearly suggests another language. Include 1-3 code blocks, each demonstrating a different aspect. Add brief comments explaining key lines. Make the code runnable and self-contained. Show expected output where relevant. Format as a section with brief prose connecting the examples.`
  }
];

// src/output-rules.ts
function getOutputRules() {
  const today = new Date().toISOString().slice(0, 10);
  return `

Formatting rules:
- The VERY FIRST characters of your output must be --- (the YAML frontmatter opening). No blank lines, no text, no whitespace before it. Output frontmatter in this exact format:
  ---
  tags: [tag1, tag2, tag3, ...]
  status: Seed
  updated: ${today}
  ---
  Generate 4-8 lowercase, hyphenated tags relevant to the topic and mode. Always include the broad domain (e.g. engineering, cs, math, dsa). Use today's date (${today}) for "updated".
- After the frontmatter closing ---, add a blank line, then a level-1 heading with the note title (e.g. # Topic Name).
- Write in markdown.
- Never use em dashes. Use commas, periods, colons, or semicolons instead. Write "to" instead of a range dash where possible.
- Write in plain, clear language. Prefer short sentences. Avoid jargon unless defining it. The reader should never have to re-read a sentence to understand it.
- Escape dollar signs used for currency with a backslash: write \\$1M not $1M, \\$500k not $500k, \\$1,000 not $1,000. Unescaped $ is interpreted as LaTeX and will corrupt surrounding text.
- Break up dense text into short paragraphs (2-4 sentences max per paragraph). Add a blank line between paragraphs.
- After each heading, start with a brief orienting sentence before diving into details.
- Use bullet points for lists, but follow each bullet with a full sentence, not just a keyword.
- Use mermaid code blocks for complex diagrams, flowcharts, architecture, processes, hierarchies, or relationships. IMPORTANT mermaid syntax rules:
  1. Each statement MUST be on its own line with a real line break. Never compress an entire diagram onto one line.
  2. For multi-line text INSIDE a single node label, use <br/> as the line break character (e.g. A["Line one<br/>Line two"]).
  3. Never use literal backslash-n characters inside mermaid blocks. Use <br/> for line breaks within labels.
  4. Always wrap node labels in double quotes inside their shape delimiters: A["Load Balancer"], B["Database"], C("Rounded"), D{"Decision"}. This is critical when labels contain parentheses, slashes, or other special characters (e.g. A["System (Restart/Scale)"], NOT A[System (Restart/Scale)]).
  5. Always wrap subgraph titles in double quotes: subgraph "Data Plane".
  6. Close every subgraph with 'end' on its own line.
  7. Use plain hyphens (-) instead of em dashes. Keep mermaid syntax strictly ASCII-safe.
  8. Never use special characters like em dashes, curly quotes, or unicode symbols outside of quoted labels.
  9. Put edge labels in double quotes with pipes: -->|"label"|.
  10. "end" is a reserved keyword in mermaid (closes subgraphs). Never use "end" as a node ID or as part of a node ID. Use alternatives like "finish", "done", "complete", or prefix it: "endNode", "processEnd". Also avoid words containing "end" as a substring in node labels or edge labels (e.g. "Send", "Backend", "Append", "endpoint"). Use synonyms: "Transmit" instead of "Send", "Server" instead of "Backend", "Add" instead of "Append".
  11. Never use markdown list syntax inside node or edge labels. Mermaid does not support markdown lists in labels and will show "Unsupported markdown: list". This includes unordered lists ("- item", "* item") AND numbered lists ("1. Step one", "1) Step one"). For numbered steps in edge labels, use a colon: -->|"1: Serialize"| not -->|"1. Serialize"|. For lists inside node labels, use <br/> to separate items.
- Use tables for comparisons or structured data.
- Use code blocks with language tags for code or config.
- Use Title Case for all headings and subheadings (e.g. "How It Works", "Key Properties and Theorems").
- Do not wrap the entire output in a markdown code fence.
- For collapsible/expandable content (hints, progressive reveals, spoilers), use Obsidian collapsible callouts instead of HTML <details> tags. Format: \`> [!tip]- Title\` (the \`-\` makes it collapsed by default). EVERY line inside a callout MUST start with \`> \`, including blank lines (\`>\`), code block content, AND the closing \`\`\` fence. Missing \`> \` on ANY line (especially the closing \`\`\`) breaks the entire callout rendering. Example:
  > [!tip]- Click to reveal
  >
  > \`\`\`python
  > def example():
  >     return True
  > \`\`\`
  >
  > More text after code.
- Do NOT use HTML <details> or <summary> tags. They break code rendering in Obsidian.
- Your output will be saved directly into an Obsidian note. Do not ask the user questions, do not add sign-off lines, do not offer to save the file, do not include any conversational text. Output only the note content.`;
}
var INLINE_OUTPUT_RULES = `

Output rules:
- Output ONLY the content to be inserted or used as replacement. No preamble, no sign-off, no explanation of what you did.
- Never use em dashes. Use commas, periods, colons, or semicolons instead.
- Write in plain, clear language. Prefer short sentences. Break up dense text with blank lines between paragraphs (2-4 sentences max per paragraph).
- Escape dollar signs used for currency: \\$1M not $1M, \\$500k not $500k. Unescaped $ is LaTeX.
- Match the formatting style (heading levels, list style, indentation) of the surrounding note.
- Write in markdown.
- Use mermaid code blocks for diagrams. Each statement on its own line. Use <br/> for line breaks within node labels (never literal backslash-n). Always wrap labels in double quotes inside shape delimiters: A["Label"], B("Rounded"), C{"Decision"}. Quoting is critical when labels contain parentheses or slashes (e.g. A["System (Restart/Scale)"]). Wrap subgraph titles in quotes. Close each subgraph with 'end'. Never use "end" as a node ID (it is reserved). Avoid words containing "end" as a substring in labels (Send, Backend, Append, endpoint); use synonyms instead. ASCII-safe only, no em dashes. Never use markdown list syntax inside labels (no "- item", "* item", "1. step", or "1) step"); for numbered steps in edge labels use a colon "1:" not "1." to avoid "Unsupported markdown: list" errors.
- For collapsible content use Obsidian callouts: \`> [!tip]- Title\`. EVERY line inside (including closing \`\`\`) MUST start with \`> \`. Do NOT use HTML <details> tags.
- Do not wrap the entire output in a markdown code fence.
- Do not include any conversational text.`;
var APPEND_OUTPUT_RULES = `

Formatting rules:
- Do NOT include YAML frontmatter or a title heading. This content will be appended to an existing note.
- Write in markdown.
- Never use em dashes. Use commas, periods, colons, or semicolons instead. Write "to" instead of a range dash where possible.
- Write in plain, clear language. Prefer short sentences. Avoid jargon unless defining it.
- Escape dollar signs used for currency: \\$1M not $1M, \\$500k not $500k. Unescaped $ is LaTeX.
- Break up dense text into short paragraphs (2-4 sentences max per paragraph). Add a blank line between paragraphs.
- Use bullet points for lists, but follow each bullet with a full sentence, not just a keyword.
- Use mermaid code blocks for diagrams. Each statement on its own line. Use <br/> for line breaks within node labels (never literal backslash-n). Always wrap labels in double quotes inside shape delimiters: A["Label"], B("Rounded"), C{"Decision"}. Quoting is critical when labels contain parentheses or slashes (e.g. A["System (Restart/Scale)"]). Wrap subgraph titles in quotes. Close each subgraph with 'end'. Never use "end" as a node ID (it is reserved). Avoid words containing "end" as a substring in labels (Send, Backend, Append, endpoint); use synonyms instead. ASCII-safe only, no em dashes. Never use markdown list syntax inside labels (no "- item", "* item", "1. step", or "1) step"); for numbered steps in edge labels use a colon "1:" not "1." to avoid "Unsupported markdown: list" errors.
- Use tables for comparisons or structured data.
- Use code blocks with language tags for code or config.
- Use Title Case for all headings and subheadings.
- For collapsible content use Obsidian callouts: \`> [!tip]- Title\`. EVERY line inside (including closing \`\`\`) MUST start with \`> \`. Do NOT use HTML <details> tags.
- Do not wrap the entire output in a markdown code fence.
- Do not include any conversational text. Output only the content to append.`;

// src/cli.ts
var import_child_process = require("child_process");
var import_os = require("os");
var spawnProcess = import_child_process.spawn;
var IS_WINDOWS = process.platform === "win32";
function quoteForCmd(arg) {
  if (arg !== "" && !/[\s"&|<>^()]/.test(arg))
    return arg;
  return `"${arg.replace(/"/g, '""')}"`;
}
function expandHome(command) {
  if (command === "~")
    return (0, import_os.homedir)();
  if (command.startsWith("~/"))
    return (0, import_os.homedir)() + command.slice(1);
  return command;
}
function spawnCli(command, args) {
  if (IS_WINDOWS) {
    return spawnProcess(quoteForCmd(command), args.map(quoteForCmd), {
      shell: true,
      windowsHide: true
    });
  }
  return spawnProcess(expandHome(command), args, {
    shell: false,
    windowsHide: true
  });
}

// src/logger.ts
var LOG_PREFIX = "[Second Brain Builder]";
var MAX_LOG_LINES = 500;
var Logger = class {
  constructor() {
    this.lines = [];
    this.enabled = true;
  }
  setEnabled(enabled) {
    this.enabled = enabled;
  }
  info(msg, ...data) {
    if (!this.enabled)
      return;
    const entry = `${this.timestamp()} INFO  ${msg}`;
    this.lines.push(entry);
    console.debug(LOG_PREFIX, msg, ...data);
    this.trim();
  }
  error(msg, ...data) {
    const entry = `${this.timestamp()} ERROR ${msg}`;
    this.lines.push(entry);
    console.error(LOG_PREFIX, msg, ...data);
    this.trim();
  }
  warn(msg, ...data) {
    if (!this.enabled)
      return;
    const entry = `${this.timestamp()} WARN  ${msg}`;
    this.lines.push(entry);
    console.warn(LOG_PREFIX, msg, ...data);
    this.trim();
  }
  getLog() {
    return this.lines.join("\n");
  }
  clear() {
    this.lines = [];
  }
  timestamp() {
    return new Date().toISOString().replace("T", " ").slice(0, 19);
  }
  trim() {
    if (this.lines.length > MAX_LOG_LINES) {
      this.lines = this.lines.slice(-MAX_LOG_LINES);
    }
  }
};
var logger = new Logger();

// src/utils.ts
var TITLE_CASE_MINOR = /* @__PURE__ */ new Set([
  "a",
  "an",
  "the",
  "and",
  "but",
  "or",
  "nor",
  "for",
  "yet",
  "so",
  "in",
  "on",
  "at",
  "to",
  "by",
  "of",
  "up",
  "as",
  "if",
  "is",
  "vs"
]);
function toTitleCase(str) {
  return str.replace(/\S+/g, (word, index) => {
    if (index === 0 || !TITLE_CASE_MINOR.has(word.toLowerCase())) {
      return word.charAt(0).toUpperCase() + word.slice(1);
    }
    return word.toLowerCase();
  });
}
function sanitizeFilename(name) {
  return toTitleCase(
    name.replace(/[\\/:*?"<>|]/g, "").replace(/\s+/g, " ").trim().slice(0, 100)
  );
}
function splitIntoTopics(text) {
  const parts = text.split(/\r?\n|[,;]/).map((p) => p.replace(/^[-*•\d.)\s]+/, "").trim()).filter(Boolean);
  const looksLikeList = parts.length > 1 && parts.every((p) => p.length <= 50 && p.split(/\s+/).length <= 6);
  return looksLikeList ? [...new Set(parts)] : [text.replace(/\s+/g, " ").trim()];
}
function extractJsonArray(raw) {
  const start = raw.indexOf("[");
  const end = raw.lastIndexOf("]");
  if (start >= 0 && end > start) {
    return raw.slice(start, end + 1);
  }
  return raw.replace(/^```(?:json)?\s*/m, "").replace(/\s*```\s*$/m, "").trim();
}
function isNoteEffectivelyEmpty(content) {
  const stripped = content.replace(/^#\s+.*$/gm, "").trim();
  return stripped.length === 0;
}

// src/fixers.ts
function fixCodeBlocks(content) {
  return content.replace(/```(\w*)\n?([\s\S]*?)```/g, (_match, lang, inner) => {
    const fixed = inner.replace(/\\n/g, "\n");
    const ending = fixed.endsWith("\n") ? "" : "\n";
    return "```" + lang + "\n" + fixed + ending + "```";
  });
}
function fixMermaidBlocks(content) {
  return content.replace(/```mermaid\n([\s\S]*?)```/g, (_match, inner) => {
    let fixed = inner.replace(/\\n/g, "\n");
    let depth = 0;
    fixed = fixed.split("\n").filter((line) => {
      const trimmed = line.trim();
      if (/^subgraph\b/.test(trimmed)) {
        depth++;
        return true;
      }
      if (trimmed === "end") {
        if (depth > 0) {
          depth--;
          return true;
        }
        return false;
      }
      return true;
    }).join("\n");
    const zwsp = "\u200B";
    fixed = fixed.split("\n").map((line) => {
      if (line.trim() === "end")
        return line;
      return line.replace(/end/g, "e" + zwsp + "nd");
    }).join("\n");
    const fixNumberedList = (label) => label.replace(/^(\d+)[.)]\s/gm, "$1: ").replace(/(<br\/?>)(\d+)[.)]\s/gi, "$1$2: ");
    const fixUnorderedList = (label) => {
      if (!/(?:^|\n)\s*[-*] /m.test(label))
        return label;
      return label.split(/\n/).map((l) => l.replace(/^\s*[-*]\s+/, "").trim()).filter((l) => l.length > 0).join("<br/>");
    };
    const fixMermaidLabel = (label) => fixNumberedList(fixUnorderedList(label));
    fixed = fixed.replace(/\|"([^"]*?)"\|/g, (_m, label) => {
      const newLabel = fixMermaidLabel(label);
      return newLabel === label ? _m : '|"' + newLabel + '"|';
    });
    fixed = fixed.replace(/\["([^\]]*?)"\]/g, (_m, label) => {
      const newLabel = fixMermaidLabel(label);
      return newLabel === label ? _m : '["' + newLabel + '"]';
    });
    fixed = fixed.replace(/\[([^\]"]*[()/\\][^\]"]*)\]/g, '["$1"]');
    fixed = fixed.replace(/\(([^)"]*[()][^)"]*)\)/g, '("$1")');
    if (!fixed.endsWith("\n"))
      fixed += "\n";
    return "```mermaid\n" + fixed + "```";
  });
}
function fixDetailsBlocks(content) {
  return content.replace(
    /<details>\s*<summary>([\s\S]*?)<\/summary>([\s\S]*?)<\/details>/g,
    (_match, title, body) => {
      let cleaned = body;
      cleaned = cleaned.replace(
        /<pre><code(?:\s+class="language-(\w+)")?>([\s\S]*?)<\/code><\/pre>/g,
        (_m, lang, code) => {
          const unescaped = code.replace(/&amp;/g, "&").replace(/&lt;/g, "<").replace(/&gt;/g, ">");
          return "```" + (lang || "") + "\n" + unescaped + "```";
        }
      );
      const trimmed = cleaned.trim();
      const lines = trimmed.split("\n");
      const prefixed = lines.map((line) => `> ${line}`).join("\n");
      return `> [!tip]- ${title.trim()}
> 
${prefixed}`;
    }
  );
}
function fixCalloutCodeFences(content) {
  const lines = content.split("\n");
  let inCallout = false;
  let inCalloutCode = false;
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    if (inCalloutCode) {
      if (/^>\s*```\s*$/.test(line)) {
        inCalloutCode = false;
      } else if (/^```\s*$/.test(line)) {
        lines[i] = "> ```";
        inCalloutCode = false;
      } else if (!/^>/.test(line) && line.trim() !== "") {
        lines[i] = "> " + line;
      }
      continue;
    }
    if (/^>\s*\[!/.test(line)) {
      inCallout = true;
      inCalloutCode = false;
    } else if (inCallout && /^>\s*```\w*/.test(line)) {
      inCalloutCode = true;
    } else if (inCallout && !/^>/.test(line) && line.trim() !== "") {
      inCallout = false;
    }
  }
  return lines.join("\n");
}
function fixCurrencyDollars(content) {
  const lines = content.split("\n");
  let inCode = false;
  for (let i = 0; i < lines.length; i++) {
    if (/^```/.test(lines[i])) {
      inCode = !inCode;
      continue;
    }
    if (inCode)
      continue;
    lines[i] = lines[i].replace(/(?<!\\)\$(\d+[A-Za-z])/g, "\\$$$1");
    lines[i] = lines[i].replace(/(?<!\\)\$(\d{1,3}(?:,\d{3})+)(?!\})/g, "\\$$$1");
    lines[i] = lines[i].replace(/(?<!\\)\$(\d+\.\d+\s+[a-zA-Z])/g, "\\$$$1");
  }
  return lines.join("\n");
}
function fixDataviewInlineQueries(content) {
  return content.replace(/`(=\s)/g, "`\u200B$1");
}

// src/modals/generation.ts
var import_obsidian2 = require("obsidian");

// src/ui.ts
var import_obsidian = require("obsidian");
function setModalTitle(modal, title) {
  if (typeof modal.setTitle === "function") {
    modal.setTitle(title);
  }
  if (modal.titleEl) {
    modal.titleEl.innerText = title;
  }
}
function renderModeCard(container, mode) {
  const card = container.createDiv({ cls: "ch-card" });
  const iconEl = card.createDiv({ cls: "ch-card-icon" });
  (0, import_obsidian.setIcon)(iconEl, mode.icon);
  const body = card.createDiv({ cls: "ch-card-body" });
  body.createDiv({ cls: "ch-card-name", text: mode.name });
  body.createDiv({ cls: "ch-card-desc", text: mode.description });
  return card;
}
function createModeGrid(wrapper, modes, onCardClick) {
  const search = wrapper.createEl("input", { type: "text", cls: "ch-search", placeholder: "Search styles..." });
  const grid = wrapper.createDiv({ cls: "ch-grid" });
  const cards = [];
  for (const mode of modes) {
    const card = renderModeCard(grid, mode);
    cards.push({ id: mode.id, el: card });
    card.addEventListener("click", () => onCardClick(mode, card));
  }
  search.addEventListener("keydown", (e) => {
    e.stopPropagation();
    if (e.key === "Enter") {
      e.preventDefault();
      const firstVisible = cards.find((c) => !c.el.hasClass("ch-hidden"));
      firstVisible == null ? void 0 : firstVisible.el.click();
    }
  });
  search.addEventListener("input", () => {
    const q = search.value.toLowerCase();
    for (const c of cards) {
      const mode = modes.find((m) => m.id === c.id);
      const match = !q || mode.name.toLowerCase().includes(q) || mode.description.toLowerCase().includes(q);
      c.el.toggleClass("ch-hidden", !match);
    }
  });
  return { grid, cards };
}

// src/modals/generation.ts
var NoteCreatorModal = class extends import_obsidian2.Modal {
  constructor(app, modes, selection, lastModeId, onSubmit, onModeUsed, subfolderDefault, parentFolderPath) {
    super(app);
    this.modes = modes;
    this.selection = String(selection || "");
    this.lastModeId = lastModeId;
    this.onSubmit = onSubmit;
    this.onModeUsed = onModeUsed || (() => {
    });
    this.selectedModes = [];
    this.titleValue = sanitizeFilename(this.selection);
    this.extraValue = "";
    this.subfolderDefault = subfolderDefault != null ? subfolderDefault : null;
    this.subfolderValue = subfolderDefault != null ? subfolderDefault : "";
    this.topicsValue = splitIntoTopics(this.selection).join("\n");
    this.parentFolderPath = parentFolderPath != null ? parentFolderPath : "";
    this.previewEl = null;
    this.btnEls = [];
    this.submitBtnEl = null;
  }
  parseTopics() {
    return [...new Set(this.topicsValue.split("\n").map((t) => sanitizeFilename(t)).filter(Boolean))];
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal");
    setModalTitle(this, this.subfolderDefault === null ? "Create note" : "Create sub-notes");
    const wrapper = el.createDiv();
    if (this.subfolderDefault !== null) {
      const intro = wrapper.createDiv({ cls: "ch-flow" });
      const sel = this.selection.length > 80 ? this.selection.slice(0, 80) + "..." : this.selection;
      intro.createDiv({ cls: "ch-flow-selection", text: `Selected: "${sel}"` });
      intro.createDiv({ text: "Each title below becomes its own note, generated using your selection and this note as context. Rename the titles to anything - the notes stay grounded in the selected text, and they link to each other and back to this note." });
    }
    wrapper.createDiv({ cls: "ch-label", text: "Note style" });
    wrapper.createDiv({ cls: "ch-hint", text: "Click multiple to blend styles into one note" });
    const { cards } = createModeGrid(wrapper, this.modes, (mode) => this.toggleMode(mode));
    this.btnEls = cards;
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    if (this.subfolderDefault === null) {
      sep.createDiv({ cls: "ch-label", text: "Note title" });
      const titleInput = sep.createEl("input", { type: "text", cls: "ch-input" });
      titleInput.value = this.titleValue;
      titleInput.addEventListener("input", () => {
        this.titleValue = titleInput.value;
      });
      titleInput.addEventListener("keydown", (e) => {
        e.stopPropagation();
        if (e.key === "Enter") {
          e.preventDefault();
          this.doSubmit();
        }
      });
    } else {
      sep.createDiv({ cls: "ch-label", text: "Sub-folder (created under this note's folder)" });
      const subfolderInput = sep.createEl("input", { type: "text", cls: "ch-input" });
      subfolderInput.value = this.subfolderValue;
      subfolderInput.placeholder = "e.g. Components, Deep Dives...";
      subfolderInput.addEventListener("input", () => {
        this.subfolderValue = subfolderInput.value;
        this.updatePreview();
      });
      subfolderInput.addEventListener("keydown", (e) => {
        e.stopPropagation();
        if (e.key === "Enter") {
          e.preventDefault();
          this.doSubmit();
        }
      });
      sep.createDiv({ cls: "ch-label", text: "Note titles (one per line)" });
      sep.createDiv({ cls: "ch-hint", text: "Each line is the exact title of one new note" });
      const topicsInput = sep.createEl("textarea", { cls: "ch-textarea" });
      topicsInput.value = this.topicsValue;
      topicsInput.rows = Math.min(8, Math.max(3, this.topicsValue.split("\n").length));
      topicsInput.addEventListener("input", () => {
        this.topicsValue = topicsInput.value;
        this.updatePreview();
        this.updateModeButtons();
      });
      topicsInput.addEventListener("keydown", (e) => {
        e.stopPropagation();
        if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
          e.preventDefault();
          this.doSubmit();
        }
      });
      this.previewEl = sep.createDiv();
      this.updatePreview();
    }
    sep.createDiv({ cls: "ch-label", text: "Extra instructions (optional)" });
    const extraInput = sep.createEl("textarea", { cls: "ch-textarea" });
    extraInput.placeholder = "e.g. Focus on practical examples, compare with X, keep it beginner-level...";
    extraInput.rows = 2;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button", { text: "Select a note style" });
    this.submitBtnEl.disabled = true;
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
    if (this.lastModeId) {
      const lastMode = this.modes.find((m) => m.id === this.lastModeId);
      if (lastMode)
        this.toggleMode(lastMode);
    }
  }
  toggleMode(mode) {
    const idx = this.selectedModes.findIndex((m) => m.id === mode.id);
    if (idx >= 0) {
      this.selectedModes.splice(idx, 1);
    } else {
      this.selectedModes.push(mode);
    }
    this.updateModeButtons();
    this.updatePreview();
  }
  updateModeButtons() {
    const selectedIds = new Set(this.selectedModes.map((m) => m.id));
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", selectedIds.has(b.id));
    }
    if (this.submitBtnEl) {
      const generateLabel = this.subfolderDefault === null ? "Generate" : `Generate ${this.parseTopics().length || "?"} sub-note${this.parseTopics().length === 1 ? "" : "s"}`;
      if (this.selectedModes.length === 0) {
        this.submitBtnEl.disabled = true;
        this.submitBtnEl.setText("Select a note style");
      } else if (this.selectedModes.length === 1) {
        this.submitBtnEl.disabled = false;
        this.submitBtnEl.setText(`${generateLabel} - ${this.selectedModes[0].name}`);
      } else {
        this.submitBtnEl.disabled = false;
        this.submitBtnEl.setText(`${generateLabel} blended - ${this.selectedModes.map((m) => m.name).join(" + ")}`);
      }
    }
  }
  updatePreview() {
    if (!this.previewEl)
      return;
    this.previewEl.empty();
    const subfolder = sanitizeFilename(this.subfolderValue);
    const dir = this.parentFolderPath ? `${this.parentFolderPath}/${subfolder || "?"}` : subfolder || "?";
    const titles = this.parseTopics();
    this.previewEl.createDiv({ cls: "ch-label", text: "What will happen" });
    if (titles.length === 0) {
      this.previewEl.createDiv({ cls: "ch-hint", text: "Nothing yet - add at least one title line above" });
      return;
    }
    const styleName = this.selectedModes.length > 0 ? this.selectedModes.map((m) => m.name).join(" + ") : "the selected style";
    const list = this.previewEl.createDiv({ cls: "ch-preview-list" });
    for (const title of titles) {
      list.createDiv({ cls: "ch-preview-item", text: `${dir}/${title}.md - a ${styleName} note about "${title}"` });
    }
    list.createDiv({ cls: "ch-preview-note", text: "Each note is written from your selected passage and the surrounding note, and links to the other notes above and back to this note." });
  }
  doSubmit() {
    if (this.selectedModes.length === 0) {
      new import_obsidian2.Notice("Pick at least one note style.");
      return;
    }
    let titles;
    let subfolder;
    if (this.subfolderDefault === null) {
      const title = sanitizeFilename(this.titleValue.trim());
      if (!title) {
        new import_obsidian2.Notice("Note title cannot be empty.");
        return;
      }
      titles = [title];
    } else {
      subfolder = sanitizeFilename(this.subfolderValue);
      if (!subfolder) {
        new import_obsidian2.Notice("Sub-folder name cannot be empty.");
        return;
      }
      titles = this.parseTopics();
      if (titles.length === 0) {
        new import_obsidian2.Notice("Add at least one sub-note title.");
        return;
      }
    }
    this.close();
    this.onModeUsed(this.selectedModes[0].id);
    let finalMode;
    if (this.selectedModes.length === 1) {
      finalMode = this.selectedModes[0];
    } else {
      finalMode = NoteCreatorModal.createBlendedMode(this.selectedModes);
    }
    this.onSubmit(titles.map((title) => ({
      mode: finalMode,
      title,
      extraInstructions: this.extraValue.trim(),
      subfolder
    })));
  }
  static createBlendedMode(modes) {
    const styleList = modes.map(
      (m, i) => `${i + 1}. **${m.name}** -${m.description}`
    ).join("\n");
    const prompt = `You are a knowledge assistant creating a note that seamlessly combines multiple note-taking styles into one cohesive document for an Obsidian vault.

The user selected "{selection}" while reading the following note:

---
{context}
---

Create a comprehensive note on "{selection}" that blends the following styles:

${styleList}

Guidelines for blending:
- Do NOT create separate sections for each style. Weave their strengths together naturally into a single cohesive note.
- Draw on the structural depth of more analytical styles and the clarity of more accessible styles.
- If one style emphasizes examples and another emphasizes theory, include both - interleaved, not segregated.
- If styles have overlapping sections (e.g. both want "Related Concepts"), merge them into one.
- Use the strongest elements from each style: analogies, diagrams, critical questions, worked examples, proofs, trade-off analysis, etc.
- Maintain a consistent tone throughout - do not shift voice between sections.
- Use mermaid diagrams, tables, code blocks, and LaTeX where appropriate.
- Link related concepts as \`[[WikiLinks]]\`.
- Use Obsidian callouts (\`> [!tip]\`, \`> [!warning]\`, \`> [!info]\`, \`> [!question]\`, etc.) where they add value.`;
    return {
      id: "blended-" + modes.map((m) => m.id).join("-"),
      name: modes.map((m) => m.name).join(" + "),
      icon: modes[0].icon,
      description: "Blended: " + modes.map((m) => m.name).join(", "),
      prompt
    };
  }
  onClose() {
    this.contentEl.empty();
  }
};
var InlineActionModal = class extends import_obsidian2.Modal {
  constructor(app, actions, selection, onSubmit, preselectedAction) {
    super(app);
    this.actions = actions;
    this.selection = selection;
    this.onSubmit = onSubmit;
    this.selectedAction = preselectedAction || null;
    this.insertionMode = "below";
    this.extraValue = "";
    this.btnEls = [];
    this.submitBtnEl = null;
    this.preselectedAction = preselectedAction || null;
    this.replaceBtn = null;
    this.belowBtn = null;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal");
    const label = this.selection.length > 60 ? this.selection.slice(0, 60) + "..." : this.selection;
    setModalTitle(this, `Enhance: "${label}"`);
    const wrapper = el.createDiv();
    if (!this.preselectedAction) {
      wrapper.createDiv({ cls: "ch-label", text: "Action" });
      const grid = wrapper.createDiv({ cls: "ch-grid ch-grid-3col" });
      this.btnEls = [];
      for (const action of this.actions) {
        const card = renderModeCard(grid, action);
        this.btnEls.push({ id: action.id, el: card });
        card.addEventListener("click", () => this.pickAction(action));
      }
    }
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    sep.createDiv({ cls: "ch-label", text: "Insertion mode" });
    const modeRow = sep.createDiv({ cls: "ch-format-row" });
    this.replaceBtn = modeRow.createEl("button", { text: "Replace selection" });
    this.belowBtn = modeRow.createEl("button", { text: "Insert below" });
    this.replaceBtn.addEventListener("click", () => {
      this.insertionMode = "replace";
      this.updateModeButtons();
    });
    this.belowBtn.addEventListener("click", () => {
      this.insertionMode = "below";
      this.updateModeButtons();
    });
    this.updateModeButtons();
    wrapper.createDiv({ cls: "ch-label", text: "Extra instructions (optional)" });
    const extraInput = wrapper.createEl("textarea", { cls: "ch-textarea" });
    extraInput.placeholder = "e.g. Keep it under 200 words, use simple analogies...";
    extraInput.rows = 2;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button");
    if (this.preselectedAction) {
      this.submitBtnEl.setText("Run - " + this.preselectedAction.name);
      this.submitBtnEl.disabled = false;
    } else {
      this.submitBtnEl.setText("Select an action");
      this.submitBtnEl.disabled = true;
    }
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
  }
  updateModeButtons() {
    if (!this.replaceBtn || !this.belowBtn)
      return;
    if (this.insertionMode === "replace") {
      this.replaceBtn.addClass("mod-cta");
      this.belowBtn.removeClass("mod-cta");
    } else {
      this.belowBtn.addClass("mod-cta");
      this.replaceBtn.removeClass("mod-cta");
    }
  }
  pickAction(action) {
    this.selectedAction = action;
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", b.id === action.id);
    }
    if (this.submitBtnEl) {
      this.submitBtnEl.disabled = false;
      this.submitBtnEl.setText("Run - " + action.name);
    }
  }
  doSubmit() {
    if (!this.selectedAction) {
      new import_obsidian2.Notice("Pick an action first.");
      return;
    }
    this.close();
    this.onSubmit({
      action: this.selectedAction,
      insertionMode: this.insertionMode,
      extraInstructions: this.extraValue.trim()
    });
  }
  onClose() {
    this.contentEl.empty();
  }
};
var FullNoteActionModal = class extends import_obsidian2.Modal {
  constructor(app, modes, fileName, onSubmit) {
    super(app);
    this.modes = modes;
    this.fileName = fileName;
    this.onSubmit = onSubmit;
    this.selectedMode = null;
    this.extraValue = "";
    this.btnEls = [];
    this.submitBtnEl = null;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal");
    setModalTitle(this, "Analyze: " + this.fileName);
    const wrapper = el.createDiv();
    wrapper.createDiv({ cls: "ch-label", text: "Action" });
    const { cards } = createModeGrid(wrapper, this.modes, (mode) => this.pickMode(mode));
    this.btnEls = cards;
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    sep.createDiv({ cls: "ch-label", text: "Extra instructions (optional)" });
    const extraInput = sep.createEl("textarea", { cls: "ch-textarea" });
    extraInput.placeholder = "e.g. Focus on open-ended questions, make exercises harder...";
    extraInput.rows = 2;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button", { text: "Select an action" });
    this.submitBtnEl.disabled = true;
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
  }
  pickMode(mode) {
    this.selectedMode = mode;
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", b.id === mode.id);
    }
    if (this.submitBtnEl) {
      this.submitBtnEl.disabled = false;
      this.submitBtnEl.setText("Analyze - " + mode.name);
    }
  }
  doSubmit() {
    if (!this.selectedMode) {
      new import_obsidian2.Notice("Pick an action first.");
      return;
    }
    this.close();
    this.onSubmit(this.selectedMode, this.extraValue.trim());
  }
  onClose() {
    this.contentEl.empty();
  }
};
var FillNoteModal = class extends import_obsidian2.Modal {
  constructor(app, modes, fileName, backlinkContext, backlinkSources, lastModeId, onSubmit, onModeUsed) {
    super(app);
    this.modes = modes;
    this.fileName = fileName;
    this.backlinkContext = backlinkContext;
    this.backlinkSources = backlinkSources;
    this.lastModeId = lastModeId;
    this.onSubmit = onSubmit;
    this.onModeUsed = onModeUsed || (() => {
    });
    this.selectedMode = null;
    this.contextValue = backlinkContext;
    this.extraValue = "";
    this.btnEls = [];
    this.submitBtnEl = null;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal");
    setModalTitle(this, "Fill: " + this.fileName);
    const wrapper = el.createDiv();
    if (this.backlinkSources.length > 0) {
      const blInfo = wrapper.createDiv({ cls: "ch-backlink-info" });
      blInfo.createDiv({ cls: "ch-label", text: `Context from ${this.backlinkSources.length} backlink${this.backlinkSources.length > 1 ? "s" : ""}:` });
      for (const src of this.backlinkSources) {
        blInfo.createDiv({ cls: "ch-hint", text: "  " + src });
      }
    } else {
      wrapper.createDiv({ cls: "ch-hint", text: "No backlinks found. Provide context below or leave empty for a general note." });
    }
    wrapper.createDiv({ cls: "ch-label", text: this.backlinkSources.length > 0 ? "Context (from backlinks, editable)" : "Context (describe what this note should cover)" });
    const ctxInput = wrapper.createEl("textarea", { cls: "ch-textarea ch-textarea-sm" });
    ctxInput.value = this.contextValue;
    ctxInput.placeholder = "e.g. This note should explain how compound interest works, with step-by-step examples...";
    ctxInput.rows = 4;
    ctxInput.addEventListener("input", () => {
      this.contextValue = ctxInput.value;
    });
    wrapper.createDiv({ cls: "ch-label", text: "Note style" });
    const { cards } = createModeGrid(wrapper, this.modes, (mode) => this.pickMode(mode));
    this.btnEls = cards;
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    sep.createDiv({ cls: "ch-label", text: "Extra instructions (optional)" });
    const extraInput = sep.createEl("textarea", { cls: "ch-textarea" });
    extraInput.placeholder = "e.g. Keep it beginner-level, include diagrams...";
    extraInput.rows = 2;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button", { text: "Select a note style" });
    this.submitBtnEl.disabled = true;
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
    if (this.lastModeId) {
      const lastMode = this.modes.find((m) => m.id === this.lastModeId);
      if (lastMode)
        this.pickMode(lastMode);
    }
  }
  pickMode(mode) {
    this.selectedMode = mode;
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", b.id === mode.id);
    }
    if (this.submitBtnEl) {
      this.submitBtnEl.disabled = false;
      this.submitBtnEl.setText("Generate - " + mode.name);
    }
  }
  doSubmit() {
    if (!this.selectedMode) {
      new import_obsidian2.Notice("Pick a note style.");
      return;
    }
    this.close();
    this.onModeUsed(this.selectedMode.id);
    this.onSubmit(this.selectedMode, this.contextValue.trim(), this.extraValue.trim());
  }
  onClose() {
    this.contentEl.empty();
  }
};

// src/modals/folder.ts
var import_obsidian3 = require("obsidian");
var TopicGeneratorModal = class extends import_obsidian3.Modal {
  constructor(app, modes, folderPath, onSubmit) {
    super(app);
    this.modes = modes;
    this.folderPath = folderPath;
    this.onSubmit = onSubmit;
    this.selectedMode = null;
    this.topicValue = "";
    this.extraValue = "";
    this.isMulti = false;
    this.btnEls = [];
    this.submitBtnEl = null;
    this.singleBtn = null;
    this.multiBtn = null;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal");
    setModalTitle(this, "Generate notes from topic");
    const wrapper = el.createDiv();
    wrapper.createDiv({ cls: "ch-hint", text: "Location: " + (this.folderPath || "/") });
    wrapper.createDiv({ cls: "ch-label", text: "Topic" });
    const topicInput = wrapper.createEl("input", { type: "text", cls: "ch-input" });
    topicInput.placeholder = "e.g. Photosynthesis, the Silk Road, compound interest...";
    topicInput.addEventListener("input", () => {
      this.topicValue = topicInput.value;
      this.updateSubmitButton();
    });
    topicInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter") {
        e.preventDefault();
        this.doSubmit();
      }
    });
    wrapper.createDiv({ cls: "ch-label", text: "Note style" });
    const { cards } = createModeGrid(wrapper, this.modes, (mode) => this.pickMode(mode));
    this.btnEls = cards;
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    sep.createDiv({ cls: "ch-label", text: "Output format" });
    const formatRow = sep.createDiv({ cls: "ch-format-row" });
    this.singleBtn = formatRow.createEl("button", { text: "Single note" });
    this.multiBtn = formatRow.createEl("button", { text: "Multi-note folder" });
    this.singleBtn.addEventListener("click", () => {
      this.isMulti = false;
      this.updateFormatButtons();
    });
    this.multiBtn.addEventListener("click", () => {
      this.isMulti = true;
      this.updateFormatButtons();
    });
    this.updateFormatButtons();
    sep.createDiv({ cls: "ch-hint", text: "Multi-note: the AI decomposes the topic into linked sub-notes in a folder" });
    wrapper.createDiv({ cls: "ch-label", text: "Extra instructions (optional)" });
    const extraInput = wrapper.createEl("textarea", { cls: "ch-textarea" });
    extraInput.placeholder = "e.g. Focus on practical examples, keep it beginner friendly...";
    extraInput.rows = 2;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button", { text: "Enter a topic and select a style" });
    this.submitBtnEl.disabled = true;
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
  }
  updateFormatButtons() {
    if (!this.singleBtn || !this.multiBtn)
      return;
    if (this.isMulti) {
      this.multiBtn.addClass("mod-cta");
      this.singleBtn.removeClass("mod-cta");
    } else {
      this.singleBtn.addClass("mod-cta");
      this.multiBtn.removeClass("mod-cta");
    }
  }
  pickMode(mode) {
    this.selectedMode = mode;
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", b.id === mode.id);
    }
    this.updateSubmitButton();
  }
  updateSubmitButton() {
    if (!this.submitBtnEl)
      return;
    if (this.topicValue.trim() && this.selectedMode) {
      this.submitBtnEl.disabled = false;
      const label = this.isMulti ? "Generate folder" : "Generate note";
      this.submitBtnEl.setText(`${label} - ${this.selectedMode.name}`);
    } else {
      this.submitBtnEl.disabled = true;
      this.submitBtnEl.setText("Enter a topic and select a style");
    }
  }
  doSubmit() {
    const topic = this.topicValue.trim();
    if (!topic) {
      new import_obsidian3.Notice("Enter a topic.");
      return;
    }
    if (!this.selectedMode) {
      new import_obsidian3.Notice("Pick a note style.");
      return;
    }
    this.close();
    this.onSubmit(topic, this.selectedMode, this.isMulti, this.extraValue.trim());
  }
  onClose() {
    this.contentEl.empty();
  }
};
var FolderGeneratorModal = class extends import_obsidian3.Modal {
  constructor(app, modes, folderPath, folderName, onSubmit) {
    super(app);
    this.modes = modes;
    this.folderPath = folderPath;
    this.folderName = folderName;
    this.onSubmit = onSubmit;
    this.selectedMode = null;
    this.scenarioValue = "";
    this.extraValue = "";
    this.btnEls = [];
    this.submitBtnEl = null;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal", "ch-modal-wide");
    setModalTitle(this, "Generate knowledge notes");
    const wrapper = el.createDiv();
    wrapper.createDiv({ cls: "ch-hint", text: `Target folder: ${this.folderPath || "/"}` });
    wrapper.createDiv({ cls: "ch-label", text: "Scenario / Prompt" });
    const scenarioInput = wrapper.createEl("textarea", { cls: "ch-textarea ch-textarea-lg" });
    scenarioInput.placeholder = "e.g. Explain how vaccines work, from immune system basics to modern types...\n\nOr: A beginner's guide to reading company financial statements...\n\nOr: The causes and consequences of the French Revolution...";
    scenarioInput.rows = 5;
    scenarioInput.addEventListener("input", () => {
      this.scenarioValue = scenarioInput.value;
      this.updateSubmitButton();
    });
    scenarioInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    wrapper.createDiv({ cls: "ch-label", text: "Note style" });
    wrapper.createDiv({ cls: "ch-hint", text: "Modes with a decomposition guide produce better multi-note breakdowns" });
    const { cards } = createModeGrid(wrapper, this.modes, (mode) => this.pickMode(mode));
    this.btnEls = cards;
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    sep.createDiv({ cls: "ch-label", text: "Extra instructions (optional)" });
    const extraInput = sep.createEl("textarea", { cls: "ch-textarea" });
    extraInput.placeholder = "e.g. Aim for expert-level depth, include real-world case studies...";
    extraInput.rows = 2;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button", { text: "Enter a scenario and select a style" });
    this.submitBtnEl.disabled = true;
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
  }
  pickMode(mode) {
    this.selectedMode = mode;
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", b.id === mode.id);
    }
    this.updateSubmitButton();
  }
  updateSubmitButton() {
    if (!this.submitBtnEl)
      return;
    if (this.scenarioValue.trim() && this.selectedMode) {
      this.submitBtnEl.disabled = false;
      this.submitBtnEl.setText(`Generate notes - ${this.selectedMode.name}`);
    } else {
      this.submitBtnEl.disabled = true;
      this.submitBtnEl.setText("Enter a scenario and select a style");
    }
  }
  doSubmit() {
    const scenario = this.scenarioValue.trim();
    if (!scenario) {
      new import_obsidian3.Notice("Enter a scenario or prompt.");
      return;
    }
    if (!this.selectedMode) {
      new import_obsidian3.Notice("Pick a note style.");
      return;
    }
    this.close();
    this.onSubmit(scenario, this.selectedMode, this.extraValue.trim());
  }
  onClose() {
    this.contentEl.empty();
  }
};
var AddTopicModal = class extends import_obsidian3.Modal {
  constructor(app, modes, folderPath, folderName, onSubmit) {
    super(app);
    this.modes = modes;
    this.folderPath = folderPath;
    this.folderName = folderName;
    this.onSubmit = onSubmit;
    this.selectedMode = null;
    this.topicValue = "";
    this.extraValue = "";
    this.btnEls = [];
    this.submitBtnEl = null;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal", "ch-modal-wide");
    setModalTitle(this, "Add topic to folder");
    const wrapper = el.createDiv();
    wrapper.createDiv({ cls: "ch-hint", text: `Folder: ${this.folderPath || "/"}` });
    wrapper.createDiv({ cls: "ch-hint", text: "Existing notes in this folder will be used as context for the new note." });
    wrapper.createDiv({ cls: "ch-label", text: "Topic to add" });
    const topicInput = wrapper.createEl("textarea", { cls: "ch-textarea" });
    topicInput.placeholder = "e.g. The placebo effect\n\nThe missing topic that should be added to this folder. A single note will be generated using existing notes as context.";
    topicInput.rows = 3;
    topicInput.addEventListener("input", () => {
      this.topicValue = topicInput.value;
      this.updateSubmitButton();
    });
    topicInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    wrapper.createDiv({ cls: "ch-label", text: "Note style" });
    const { cards } = createModeGrid(wrapper, this.modes, (mode) => this.pickMode(mode));
    this.btnEls = cards;
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    sep.createDiv({ cls: "ch-label", text: "Extra instructions (optional)" });
    const extraInput = sep.createEl("textarea", { cls: "ch-textarea" });
    extraInput.placeholder = "e.g. Focus on the practical implications, include memorable examples...";
    extraInput.rows = 2;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button", { text: "Enter a topic and select a style" });
    this.submitBtnEl.disabled = true;
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
  }
  pickMode(mode) {
    this.selectedMode = mode;
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", b.id === mode.id);
    }
    this.updateSubmitButton();
  }
  updateSubmitButton() {
    if (!this.submitBtnEl)
      return;
    if (this.topicValue.trim() && this.selectedMode) {
      this.submitBtnEl.disabled = false;
      this.submitBtnEl.setText(`Add note - ${this.selectedMode.name}`);
    } else {
      this.submitBtnEl.disabled = true;
      this.submitBtnEl.setText("Enter a topic and select a style");
    }
  }
  doSubmit() {
    const topic = this.topicValue.trim();
    if (!topic) {
      new import_obsidian3.Notice("Enter a topic.");
      return;
    }
    if (!this.selectedMode) {
      new import_obsidian3.Notice("Pick a note style.");
      return;
    }
    this.close();
    this.onSubmit(topic, this.selectedMode, this.extraValue.trim());
  }
  onClose() {
    this.contentEl.empty();
  }
};
var ExpandFolderModal = class extends import_obsidian3.Modal {
  constructor(app, modes, folderPath, folderName, existingNotes, onSubmit) {
    super(app);
    this.modes = modes;
    this.folderPath = folderPath;
    this.folderName = folderName;
    this.existingNotes = existingNotes;
    this.onSubmit = onSubmit;
    this.selectedMode = null;
    this.directionValue = "";
    this.extraValue = "";
    this.btnEls = [];
    this.submitBtnEl = null;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal", "ch-modal-wide");
    setModalTitle(this, "Expand folder with more notes");
    const wrapper = el.createDiv();
    wrapper.createDiv({ cls: "ch-hint", text: `Folder: ${this.folderPath || "/"}` });
    wrapper.createDiv({ cls: "ch-hint", text: `${this.existingNotes.length} existing notes will be read as context. New notes will not repeat existing content.` });
    if (this.existingNotes.length > 0) {
      const existingEl = wrapper.createDiv({ cls: "ch-existing-list" });
      existingEl.createEl("strong", { text: "Existing: " });
      existingEl.createSpan({ text: this.existingNotes.join(", ") });
    }
    wrapper.createDiv({ cls: "ch-label", text: "What to add" });
    const dirInput = wrapper.createEl("textarea", { cls: "ch-textarea ch-textarea-md" });
    dirInput.placeholder = "e.g. Add notes covering common misconceptions and frequently asked questions\n\nOr: Expand with real-world case studies and historical context\n\nOr: Add notes on the practical applications I haven't covered yet";
    dirInput.rows = 4;
    dirInput.addEventListener("input", () => {
      this.directionValue = dirInput.value;
      this.updateSubmitButton();
    });
    dirInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    wrapper.createDiv({ cls: "ch-label", text: "Note style" });
    wrapper.createDiv({ cls: "ch-hint", text: "Can be different from the style used for existing notes" });
    const { cards } = createModeGrid(wrapper, this.modes, (mode) => this.pickMode(mode));
    this.btnEls = cards;
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    sep.createDiv({ cls: "ch-label", text: "Extra instructions (optional)" });
    const extraInput = sep.createEl("textarea", { cls: "ch-textarea" });
    extraInput.placeholder = "e.g. Aim for expert-level depth, include concrete examples...";
    extraInput.rows = 2;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button", { text: "Describe what to add and select a style" });
    this.submitBtnEl.disabled = true;
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
  }
  pickMode(mode) {
    this.selectedMode = mode;
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", b.id === mode.id);
    }
    this.updateSubmitButton();
  }
  updateSubmitButton() {
    if (!this.submitBtnEl)
      return;
    if (this.directionValue.trim() && this.selectedMode) {
      this.submitBtnEl.disabled = false;
      this.submitBtnEl.setText(`Expand folder - ${this.selectedMode.name}`);
    } else {
      this.submitBtnEl.disabled = true;
      this.submitBtnEl.setText("Describe what to add and select a style");
    }
  }
  doSubmit() {
    const direction = this.directionValue.trim();
    if (!direction) {
      new import_obsidian3.Notice("Describe what notes to add.");
      return;
    }
    if (!this.selectedMode) {
      new import_obsidian3.Notice("Pick a note style.");
      return;
    }
    this.close();
    this.onSubmit(direction, this.selectedMode, this.extraValue.trim());
  }
  onClose() {
    this.contentEl.empty();
  }
};

// src/modals/analysis.ts
var import_obsidian4 = require("obsidian");
var FolderAnalysisModal = class extends import_obsidian4.Modal {
  constructor(app, modes, folderPath, folderName, onSubmit) {
    super(app);
    this.modes = modes;
    this.folderPath = folderPath;
    this.folderName = folderName;
    this.onSubmit = onSubmit;
    this.selectedMode = null;
    this.extraValue = "";
    this.btnEls = [];
    this.submitBtnEl = null;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal", "ch-modal-wide");
    setModalTitle(this, "Analyze folder");
    const wrapper = el.createDiv();
    wrapper.createDiv({ cls: "ch-hint", text: `Analyzing: ${this.folderPath || "/"}` });
    wrapper.createDiv({ cls: "ch-hint", text: "All notes in this folder will be read as context." });
    wrapper.createDiv({ cls: "ch-label", text: "Analysis mode" });
    const { cards } = createModeGrid(wrapper, this.modes, (mode) => this.pickMode(mode));
    this.btnEls = cards;
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    sep.createDiv({ cls: "ch-label", text: "Additional content (articles, context, instructions)" });
    const extraInput = sep.createEl("textarea", { cls: "ch-textarea ch-textarea-xl" });
    extraInput.placeholder = "Paste articles, blog posts, or extra context here...\n\nOptional for most modes: leave empty to analyze the existing notes as they are.";
    extraInput.rows = 8;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button", { text: "Select an analysis mode" });
    this.submitBtnEl.disabled = true;
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
  }
  pickMode(mode) {
    this.selectedMode = mode;
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", b.id === mode.id);
    }
    if (this.submitBtnEl) {
      this.submitBtnEl.disabled = false;
      this.submitBtnEl.setText(`Analyze - ${mode.name}`);
    }
  }
  doSubmit() {
    if (!this.selectedMode) {
      new import_obsidian4.Notice("Pick an analysis mode.");
      return;
    }
    this.close();
    this.onSubmit(this.selectedMode, this.extraValue.trim());
  }
  onClose() {
    this.contentEl.empty();
  }
};
var NoteAnalysisModal = class extends import_obsidian4.Modal {
  constructor(app, modes, file, onSubmit) {
    super(app);
    this.modes = modes;
    this.file = file;
    this.onSubmit = onSubmit;
    this.selectedMode = null;
    this.extraValue = "";
    this.btnEls = [];
    this.submitBtnEl = null;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal", "ch-modal-wide");
    setModalTitle(this, "Analyze note");
    const wrapper = el.createDiv();
    wrapper.createDiv({ cls: "ch-hint", text: `Analyzing: ${this.file.basename}` });
    wrapper.createDiv({ cls: "ch-label", text: "Analysis mode" });
    const { cards } = createModeGrid(wrapper, this.modes, (mode) => this.pickMode(mode));
    this.btnEls = cards;
    const sep = wrapper.createDiv({ cls: "ch-sep" });
    sep.createDiv({ cls: "ch-label", text: "Additional content (articles, context, instructions)" });
    const extraInput = sep.createEl("textarea", { cls: "ch-textarea ch-textarea-xl" });
    extraInput.placeholder = "Paste articles, blog posts, or extra context here...\n\nOptional for most modes: leave empty to analyze the note as it is.";
    extraInput.rows = 8;
    extraInput.addEventListener("input", () => {
      this.extraValue = extraInput.value;
    });
    extraInput.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        this.doSubmit();
      }
    });
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    this.submitBtnEl = btnRow.createEl("button", { text: "Select an analysis mode" });
    this.submitBtnEl.disabled = true;
    this.submitBtnEl.addClass("mod-cta");
    this.submitBtnEl.addEventListener("click", () => this.doSubmit());
  }
  pickMode(mode) {
    this.selectedMode = mode;
    for (const b of this.btnEls) {
      b.el.toggleClass("is-selected", b.id === mode.id);
    }
    if (this.submitBtnEl) {
      this.submitBtnEl.disabled = false;
      this.submitBtnEl.setText(`Analyze - ${mode.name}`);
    }
  }
  doSubmit() {
    if (!this.selectedMode) {
      new import_obsidian4.Notice("Pick an analysis mode.");
      return;
    }
    this.close();
    this.onSubmit(this.selectedMode, this.extraValue.trim());
  }
  onClose() {
    this.contentEl.empty();
  }
};

// src/modals/queue.ts
var import_obsidian5 = require("obsidian");
var QueueStatusModal = class extends import_obsidian5.Modal {
  constructor(app, plugin) {
    super(app);
    this.intervalId = null;
    this.previewEl = null;
    this.timerEl = null;
    this.queueListEl = null;
    this.failedListEl = null;
    this.completedListEl = null;
    this.headerEl = null;
    this.plugin = plugin;
  }
  onOpen() {
    this.buildLayout();
    this.intervalId = window.setInterval(() => this.refresh(), 500);
  }
  buildLayout() {
    const el = this.contentEl;
    el.empty();
    setModalTitle(this, "Generation queue");
    const wrapper = el.createDiv({ cls: "ch-queue-wrapper" });
    this.headerEl = wrapper.createDiv({ cls: "ch-queue-header" });
    this.timerEl = wrapper.createDiv({ cls: "ch-queue-timer" });
    wrapper.createDiv({ cls: "ch-section-label", text: "Live output" });
    this.previewEl = wrapper.createEl("pre", { cls: "ch-queue-preview" });
    this.queueListEl = wrapper.createDiv({ cls: "ch-queue-list" });
    this.failedListEl = wrapper.createDiv({ cls: "ch-queue-list" });
    this.completedListEl = wrapper.createDiv({ cls: "ch-queue-list" });
    const btnContainer = wrapper.createDiv({ cls: "ch-queue-actions" });
    const clearBtn = btnContainer.createEl("button", { text: "Clear pending queue" });
    clearBtn.addEventListener("click", () => {
      this.plugin.clearQueue();
      this.refresh();
    });
    const retryAllBtn = btnContainer.createEl("button", { text: "Retry all failed" });
    retryAllBtn.addEventListener("click", () => {
      this.plugin.retryFailed();
      this.refresh();
    });
    this.refresh();
  }
  getItemLabelLocal(item) {
    if (item.type === "note")
      return item.noteName;
    if (item.type === "append")
      return item.fileName;
    if (item.type === "topic-note")
      return item.noteName;
    return `[${item.action.name}] inline`;
  }
  getItemModeName(item) {
    if (item.type === "note")
      return item.mode.name;
    if (item.type === "append")
      return item.mode.name;
    if (item.type === "topic-note")
      return item.mode.name;
    return item.action.name;
  }
  refresh() {
    const processing = this.plugin.getProcessingItem();
    const streamData = this.plugin.getStreamData();
    const queue = this.plugin.getQueue();
    if (this.headerEl) {
      if (processing) {
        this.headerEl.setText("");
        this.headerEl.createEl("h3", {
          cls: "ch-queue-title",
          text: "Generating: " + this.getItemLabelLocal(processing)
        });
        this.headerEl.createSpan({ cls: "ch-badge", text: this.getItemModeName(processing) });
      } else {
        this.headerEl.setText("");
        this.headerEl.createEl("h3", { cls: "ch-queue-title", text: "Queue idle" });
      }
    }
    if (this.timerEl) {
      if (processing && streamData.startTime > 0) {
        const elapsed = Math.floor((Date.now() - streamData.startTime) / 1e3);
        const mins = Math.floor(elapsed / 60);
        const secs = elapsed % 60;
        const timeStr = mins > 0 ? `${mins}m ${secs}s` : `${secs}s`;
        const chars = streamData.currentOutput.length;
        this.timerEl.setText(`Elapsed: ${timeStr} | ${chars} chars received`);
      } else {
        this.timerEl.setText("");
      }
    }
    if (this.previewEl) {
      if (processing) {
        const output = streamData.currentOutput;
        if (output.length > 0) {
          const tail = output.length > 2e3 ? "...\n" + output.slice(-2e3) : output;
          this.previewEl.setText(tail);
        } else {
          this.previewEl.setText(`Waiting for ${PROVIDER_LABELS[this.plugin.settings.aiProvider]} to respond...`);
        }
        this.previewEl.scrollTop = this.previewEl.scrollHeight;
      } else {
        this.previewEl.setText("No active generation.");
      }
    }
    if (this.queueListEl) {
      this.queueListEl.setText("");
      if (queue.length > 0) {
        this.queueListEl.createDiv({ cls: "ch-section-label", text: "Up next (" + queue.length + ")" });
        for (let i = 0; i < queue.length; i++) {
          const q = queue[i];
          const row = this.queueListEl.createDiv({ cls: "ch-queue-row" });
          row.createSpan({
            cls: "ch-row-name",
            text: i + 1 + ". " + this.getItemLabelLocal(q) + " - " + this.getItemModeName(q)
          });
          const removeBtn = row.createEl("button", { cls: "ch-btn-small", text: "x" });
          removeBtn.addEventListener("click", () => {
            this.plugin.removeFromQueue(i);
            this.refresh();
          });
        }
      }
    }
    const failedItems = this.plugin.getFailedItems();
    if (this.failedListEl) {
      this.failedListEl.setText("");
      if (failedItems.length > 0) {
        this.failedListEl.createDiv({
          cls: "ch-section-label ch-label-error",
          text: "Failed (" + failedItems.length + ")"
        });
        for (let i = 0; i < failedItems.length; i++) {
          const f = failedItems[i];
          const row = this.failedListEl.createDiv({ cls: "ch-queue-row" });
          const itemName = this.getItemLabelLocal(f.item);
          const shortErr = f.error.length > 60 ? f.error.slice(0, 60) + "..." : f.error;
          row.createSpan({ cls: "ch-row-name ch-row-name-muted", text: itemName + " - " + shortErr });
          const retryBtn = row.createEl("button", { cls: "ch-btn-small", text: "Retry" });
          retryBtn.addEventListener("click", () => {
            this.plugin.retryOne(i);
            this.refresh();
          });
          const dismissBtn = row.createEl("button", { cls: "ch-btn-small", text: "x" });
          dismissBtn.addEventListener("click", () => {
            this.plugin.dismissFailed(i);
            this.refresh();
          });
        }
      }
    }
    const completedItems = this.plugin.getCompletedItems();
    if (this.completedListEl) {
      this.completedListEl.setText("");
      if (completedItems.length > 0) {
        this.completedListEl.createDiv({
          cls: "ch-section-label ch-label-success",
          text: "Completed (" + completedItems.length + ")"
        });
        const shown = completedItems.slice(-20).reverse();
        for (const c of shown) {
          const row = this.completedListEl.createDiv({ cls: "ch-completed-row" });
          row.createSpan({ cls: "ch-row-name", text: this.getItemLabelLocal(c.item) });
          const secs = Math.round(c.elapsed / 1e3);
          row.createSpan({ cls: "ch-row-stats", text: `${(c.chars / 1e3).toFixed(1)}k chars, ${secs}s` });
        }
        if (completedItems.length > 20) {
          this.completedListEl.createDiv({
            cls: "ch-more-note",
            text: `...and ${completedItems.length - 20} more`
          });
        }
      }
    }
  }
  onClose() {
    if (this.intervalId !== null) {
      window.clearInterval(this.intervalId);
    }
    this.contentEl.empty();
  }
};
var LogViewerModal = class extends import_obsidian5.Modal {
  constructor(app) {
    super(app);
    this.interval = null;
    this.pre = null;
    this.lastLen = -1;
  }
  onOpen() {
    const { contentEl } = this;
    contentEl.addClass("ch-modal");
    setModalTitle(this, "Second Brain Builder logs");
    const btnRow = contentEl.createDiv({ cls: "ch-log-btn-row" });
    const copyBtn = btnRow.createEl("button", { text: "Copy to clipboard" });
    copyBtn.onclick = () => {
      void navigator.clipboard.writeText(logger.getLog());
      new import_obsidian5.Notice("Logs copied to clipboard");
    };
    const clearBtn = btnRow.createEl("button", { text: "Clear logs" });
    clearBtn.onclick = () => {
      logger.clear();
      this.lastLen = -1;
      this.refresh();
      new import_obsidian5.Notice("Logs cleared");
    };
    this.pre = contentEl.createEl("pre", { cls: "ch-log-pre" });
    this.refresh();
    this.interval = window.setInterval(() => this.refresh(), 1e3);
  }
  refresh() {
    if (!this.pre)
      return;
    const text = logger.getLog();
    if (text.length === this.lastLen)
      return;
    this.lastLen = text.length;
    this.pre.textContent = text || "No log entries yet.";
    this.pre.scrollTop = this.pre.scrollHeight;
  }
  onClose() {
    if (this.interval !== null)
      window.clearInterval(this.interval);
    this.contentEl.empty();
  }
};

// src/modals/system-design.ts
var import_obsidian6 = require("obsidian");
var ScaleCalculatorModal = class extends import_obsidian6.Modal {
  constructor(app, onInsert) {
    super(app);
    this.onInsert = onInsert;
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal");
    setModalTitle(this, "Scale estimation calculator");
    const wrapper = el.createDiv();
    wrapper.createDiv({ cls: "ch-hint", text: "Enter values to generate a scale estimation table for your system design note." });
    const fields = [
      { label: "Daily Active Users (DAU)", key: "dau", placeholder: "e.g. 50000000", default: "" },
      { label: "Requests per user per day", key: "rpud", placeholder: "e.g. 20", default: "10" },
      { label: "Average read payload (KB)", key: "readKb", placeholder: "e.g. 5", default: "2" },
      { label: "Average write payload (KB)", key: "writeKb", placeholder: "e.g. 50", default: "10" },
      { label: "Read:Write ratio", key: "rwRatio", placeholder: "e.g. 10:1", default: "10:1" },
      { label: "Data retention (years)", key: "retention", placeholder: "e.g. 5", default: "5" }
    ];
    const inputs = {};
    for (const f of fields) {
      wrapper.createDiv({ cls: "ch-label", text: f.label });
      const input = wrapper.createEl("input", { type: "text", cls: "ch-input" });
      input.placeholder = f.placeholder;
      input.value = f.default;
      inputs[f.key] = input;
    }
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    const btn = btnRow.createEl("button", { text: "Generate & insert" });
    btn.addClass("mod-cta");
    btn.addEventListener("click", () => {
      const dau = parseFloat(inputs.dau.value) || 0;
      const rpud = parseFloat(inputs.rpud.value) || 10;
      const readKb = parseFloat(inputs.readKb.value) || 2;
      const writeKb = parseFloat(inputs.writeKb.value) || 10;
      const retention = parseFloat(inputs.retention.value) || 5;
      const rwParts = (inputs.rwRatio.value || "10:1").split(":");
      const readRatio = parseFloat(rwParts[0]) || 10;
      const writeRatio = parseFloat(rwParts[1]) || 1;
      const totalRatio = readRatio + writeRatio;
      if (dau <= 0) {
        new import_obsidian6.Notice("Enter a valid DAU value.");
        return;
      }
      const totalReqDay = dau * rpud;
      const avgQps = totalReqDay / 86400;
      const peakQps = avgQps * 3;
      const readQps = avgQps * (readRatio / totalRatio);
      const writeQps = avgQps * (writeRatio / totalRatio);
      const readBwMbps = readQps * readKb / 1024;
      const writeBwMbps = writeQps * writeKb / 1024;
      const storagePerDayGb = writeQps * writeKb * 86400 / (1024 * 1024);
      const storagePerYearTb = storagePerDayGb * 365 / 1024;
      const storageTotalTb = storagePerYearTb * retention;
      const fmt = (n, d = 0) => {
        if (n >= 1e9)
          return (n / 1e9).toFixed(1) + "B";
        if (n >= 1e6)
          return (n / 1e6).toFixed(1) + "M";
        if (n >= 1e3)
          return (n / 1e3).toFixed(1) + "K";
        return n.toFixed(d);
      };
      const md = `## Scale Estimation

| Metric | Value |
|---|---|
| DAU | ${fmt(dau)} |
| Requests/day | ${fmt(totalReqDay)} |
| Average QPS | ${fmt(avgQps, 0)} |
| Peak QPS (3x) | ${fmt(peakQps, 0)} |
| Read QPS | ${fmt(readQps, 0)} |
| Write QPS | ${fmt(writeQps, 0)} |
| Read bandwidth | ${readBwMbps.toFixed(1)} MB/s |
| Write bandwidth | ${writeBwMbps.toFixed(1)} MB/s |
| Storage/day | ${storagePerDayGb.toFixed(1)} GB |
| Storage/year | ${storagePerYearTb.toFixed(2)} TB |
| Storage (${retention}yr) | ${storageTotalTb.toFixed(2)} TB |

**Assumptions:** ${fmt(dau)} DAU, ${rpud} req/user/day, ${readKb}KB reads, ${writeKb}KB writes, ${readRatio}:${writeRatio} read:write ratio, ${retention}yr retention. Peak = 3x average.
`;
      this.close();
      this.onInsert(md);
    });
  }
  onClose() {
    this.contentEl.empty();
  }
};
var ScaffoldWorkspaceModal = class extends import_obsidian6.Modal {
  constructor(app, onSubmit) {
    super(app);
    this.onSubmit = onSubmit;
    this.selectedSections = /* @__PURE__ */ new Set(["Systems", "Patterns", "Components", "Failures", "Tradeoffs", "Simulations", "Glossary"]);
  }
  onOpen() {
    const el = this.contentEl;
    el.empty();
    this.modalEl.addClass("ch-modal");
    setModalTitle(this, "Scaffold system design workspace");
    const wrapper = el.createDiv();
    wrapper.createDiv({ cls: "ch-hint", text: "Creates a folder hierarchy for organizing system design notes with an index note." });
    wrapper.createDiv({ cls: "ch-label", text: "Workspace name" });
    const nameInput = wrapper.createEl("input", { type: "text", cls: "ch-input" });
    nameInput.value = "System Design";
    nameInput.placeholder = "e.g. System Design, Interview Prep, Architecture";
    wrapper.createDiv({ cls: "ch-label", text: "Sections to create" });
    const sections = [
      { id: "Systems", desc: "Full system design notes (YouTube, WhatsApp, Uber)" },
      { id: "Patterns", desc: "Design patterns (Saga, CQRS, Event Sourcing)" },
      { id: "Components", desc: "Technology deep-dives (Redis, Kafka, Postgres)" },
      { id: "Failures", desc: "Failure modes (Cache Stampede, Split Brain)" },
      { id: "Tradeoffs", desc: "Technology comparisons (Postgres vs Cassandra)" },
      { id: "Simulations", desc: "Request flows and path traces" },
      { id: "Glossary", desc: "Term definitions and concept maps" }
    ];
    const checkboxes = [];
    const grid = wrapper.createDiv({ cls: "ch-grid" });
    for (const sec of sections) {
      const card = grid.createDiv({ cls: "ch-card is-selected" });
      const cb = card.createEl("input", { type: "checkbox", cls: "ch-card-checkbox" });
      cb.checked = true;
      const body = card.createDiv({ cls: "ch-card-body" });
      body.createDiv({ cls: "ch-card-name", text: sec.id });
      body.createDiv({ cls: "ch-card-desc", text: sec.desc });
      checkboxes.push({ id: sec.id, cb });
      const toggle = () => {
        cb.checked = !cb.checked;
        if (cb.checked) {
          this.selectedSections.add(sec.id);
          card.addClass("is-selected");
        } else {
          this.selectedSections.delete(sec.id);
          card.removeClass("is-selected");
        }
      };
      card.addEventListener("click", (e) => {
        if (e.target !== cb)
          toggle();
      });
      cb.addEventListener("change", () => {
        if (cb.checked) {
          this.selectedSections.add(sec.id);
          card.addClass("is-selected");
        } else {
          this.selectedSections.delete(sec.id);
          card.removeClass("is-selected");
        }
      });
    }
    const btnRow = wrapper.createDiv({ cls: "ch-btn-row" });
    const btn = btnRow.createEl("button", { text: "Create workspace" });
    btn.addClass("mod-cta");
    btn.addEventListener("click", () => {
      const name = nameInput.value.trim();
      if (!name) {
        new import_obsidian6.Notice("Enter a workspace name.");
        return;
      }
      if (this.selectedSections.size === 0) {
        new import_obsidian6.Notice("Select at least one section.");
        return;
      }
      this.close();
      void this.onSubmit(name, Array.from(this.selectedSections));
    });
  }
  onClose() {
    this.contentEl.empty();
  }
};

// src/settings-tab.ts
var import_obsidian7 = require("obsidian");
var ClaudeExplainerSettingTab = class extends import_obsidian7.PluginSettingTab {
  constructor(app, plugin) {
    super(app, plugin);
    this.plugin = plugin;
  }
  renderSetupGuide(containerEl, heading, steps) {
    new import_obsidian7.Setting(containerEl).setName(heading).setHeading();
    const guideEl = containerEl.createDiv({ cls: "ch-guide" });
    for (const step of steps) {
      const stepEl = guideEl.createDiv({ cls: "ch-guide-step" });
      if (step.title) {
        stepEl.createEl("strong", { cls: "ch-strong-block ch-gap-above", text: step.title });
        stepEl.createSpan({ text: step.text });
      } else {
        stepEl.createEl("code", { cls: "ch-guide-code", text: step.text });
      }
    }
  }
  cliTestStep(cli) {
    return {
      title: "4. Test",
      text: `Select text in any note and run "Explain selection with AI" from the command palette. If the plugin reports the CLI is not found, enter its full path above (find it with "where ${cli}" on Windows or "which ${cli}" on macOS/Linux).`
    };
  }
  display() {
    const { containerEl } = this;
    containerEl.empty();
    new import_obsidian7.Setting(containerEl).setName("AI provider").setDesc("Which AI backend to use for generating notes.").addDropdown((dropdown) => {
      dropdown.addOption("claude", "Claude");
      dropdown.addOption("gemini", "Gemini");
      dropdown.addOption("codex", "Codex");
      dropdown.addOption("ollama", "Ollama (local)");
      dropdown.setValue(this.plugin.settings.aiProvider);
      dropdown.onChange(async (value) => {
        this.plugin.settings.aiProvider = value;
        await this.plugin.saveSettings();
        this.display();
      });
    });
    if (this.plugin.settings.aiProvider === "claude") {
      new import_obsidian7.Setting(containerEl).setName("Claude CLI path").setDesc("Path to the Claude Code CLI executable.").addText(
        (text) => text.setPlaceholder("claude").setValue(this.plugin.settings.claudePath).onChange(async (value) => {
          this.plugin.settings.claudePath = value || "claude";
          await this.plugin.saveSettings();
        })
      );
      new import_obsidian7.Setting(containerEl).setName("Claude model").setDesc("Model to use (e.g. sonnet, opus, claude-sonnet-4-6). Leave empty for sonnet, which keeps token use low.").addText(
        (text) => text.setPlaceholder("sonnet").setValue(this.plugin.settings.modelFlag).onChange(async (value) => {
          this.plugin.settings.modelFlag = value.trim();
          await this.plugin.saveSettings();
        })
      );
      this.renderSetupGuide(containerEl, "Claude Code setup guide", [
        { title: "1. Install Node.js", text: "Download version 16 or newer from nodejs.org. Verify in a terminal with: node --version" },
        { title: "2. Install the Claude Code CLI", text: "In a terminal, run:" },
        { title: "", text: "npm install -g @anthropic-ai/claude-code" },
        { title: "3. Log in", text: "Run claude once in a terminal and follow the login prompts. Requires a Claude account (Pro, Max, or API billing)." },
        this.cliTestStep("claude")
      ]);
    } else if (this.plugin.settings.aiProvider === "gemini") {
      new import_obsidian7.Setting(containerEl).setName("Gemini CLI path").setDesc("Path to the Gemini CLI executable.").addText(
        (text) => text.setPlaceholder("gemini").setValue(this.plugin.settings.geminiPath).onChange(async (value) => {
          this.plugin.settings.geminiPath = value || "gemini";
          await this.plugin.saveSettings();
        })
      );
      new import_obsidian7.Setting(containerEl).setName("Gemini model").setDesc("Optional model override (e.g. gemini-2.5-pro). Leave empty for default.").addText(
        (text) => text.setPlaceholder("").setValue(this.plugin.settings.geminiModel).onChange(async (value) => {
          this.plugin.settings.geminiModel = value.trim();
          await this.plugin.saveSettings();
        })
      );
      this.renderSetupGuide(containerEl, "Gemini CLI setup guide", [
        { title: "1. Install Node.js", text: "Download version 16 or newer from nodejs.org. Verify in a terminal with: node --version" },
        { title: "2. Install the Gemini CLI", text: "In a terminal, run:" },
        { title: "", text: "npm install -g @google/gemini-cli" },
        { title: "3. Log in", text: "Run gemini once in a terminal and sign in with your Google account. The free tier is enough to start." },
        this.cliTestStep("gemini")
      ]);
    } else if (this.plugin.settings.aiProvider === "codex") {
      new import_obsidian7.Setting(containerEl).setName("Codex CLI path").setDesc("Path to the OpenAI Codex CLI executable.").addText(
        (text) => text.setPlaceholder("codex").setValue(this.plugin.settings.codexPath).onChange(async (value) => {
          this.plugin.settings.codexPath = value || "codex";
          await this.plugin.saveSettings();
        })
      );
      new import_obsidian7.Setting(containerEl).setName("Codex model").setDesc("Optional model override (e.g. gpt-5.1-codex). Leave empty for default.").addText(
        (text) => text.setPlaceholder("").setValue(this.plugin.settings.codexModel).onChange(async (value) => {
          this.plugin.settings.codexModel = value.trim();
          await this.plugin.saveSettings();
        })
      );
      this.renderSetupGuide(containerEl, "Codex CLI setup guide", [
        { title: "1. Install Node.js", text: "Download version 16 or newer from nodejs.org. Verify in a terminal with: node --version" },
        { title: "2. Install the Codex CLI", text: "In a terminal, run:" },
        { title: "", text: "npm install -g @openai/codex" },
        { title: "3. Log in", text: "Run codex once in a terminal and sign in with your OpenAI or ChatGPT account." },
        this.cliTestStep("codex")
      ]);
    } else if (this.plugin.settings.aiProvider === "ollama") {
      new import_obsidian7.Setting(containerEl).setName("Ollama server URL").setDesc("Base URL of the Ollama API server.").addText(
        (text) => text.setPlaceholder("http://localhost:11434").setValue(this.plugin.settings.ollamaUrl).onChange(async (value) => {
          this.plugin.settings.ollamaUrl = value || "http://localhost:11434";
          await this.plugin.saveSettings();
        })
      );
      const ollamaModels = [
        { value: "qwen3.5:latest", label: "Qwen 3.5 (6.6 GB)", desc: "Fast general-purpose model. Best for quick explanations, summaries, and everyday note generation. Good balance of speed and quality." },
        { value: "gemma4:e4b", label: "Gemma 4 E4B (9.6 GB)", desc: "Strong reasoning and instruction following. Best for detailed technical notes, code walkthroughs, and structured content that needs precise formatting." },
        { value: "gpt-oss:20b", label: "GPT-OSS 20B (13 GB)", desc: "Largest and most capable. Best for complex topics requiring deep analysis, multi-step reasoning, and comprehensive coverage." }
      ];
      const currentModel = this.plugin.settings.ollamaModel;
      const isPreset = ollamaModels.some((m) => m.value === currentModel);
      new import_obsidian7.Setting(containerEl).setName("Ollama model").setDesc("Select a recommended model or enter a custom one. Models must be pulled first with 'ollama pull <model>'.").addDropdown((dropdown) => {
        for (const m of ollamaModels) {
          dropdown.addOption(m.value, m.label);
        }
        dropdown.addOption("__custom__", "Custom model...");
        dropdown.setValue(isPreset ? currentModel : "__custom__");
        dropdown.onChange(async (value) => {
          if (value === "__custom__") {
            this.plugin.settings.ollamaModel = currentModel;
          } else {
            this.plugin.settings.ollamaModel = value;
          }
          await this.plugin.saveSettings();
          this.display();
        });
      });
      if (!isPreset) {
        new import_obsidian7.Setting(containerEl).setName("Custom model name").setDesc("Enter any model available in your Ollama installation.").addText(
          (text) => text.setPlaceholder("llama3").setValue(this.plugin.settings.ollamaModel).onChange(async (value) => {
            this.plugin.settings.ollamaModel = value.trim() || "llama3";
            await this.plugin.saveSettings();
          })
        );
      }
      const activeModel = ollamaModels.find((m) => m.value === this.plugin.settings.ollamaModel);
      if (activeModel) {
        const recEl = containerEl.createDiv({ cls: "setting-item ch-rec-card" });
        recEl.createEl("strong", { cls: "ch-strong-block ch-gap-below", text: activeModel.label });
        recEl.createSpan({ cls: "ch-muted-note", text: activeModel.desc });
      }
      new import_obsidian7.Setting(containerEl).setName("Model comparison").setHeading();
      const table = containerEl.createEl("table", { cls: "ch-settings-table" });
      const thead = table.createEl("thead");
      const headerRow = thead.createEl("tr");
      for (const h of ["Model", "Size", "Speed", "Best for"]) {
        headerRow.createEl("th", { text: h });
      }
      const tbody = table.createEl("tbody");
      const rows = [
        ["Qwen 3.5", "6.6 GB", "Fastest", "Quick notes, summaries, everyday use"],
        ["Gemma 4 E4B", "9.6 GB", "Medium", "Technical docs, code analysis, structured output"],
        ["GPT-OSS 20B", "13 GB", "Slowest", "Deep analysis, complex reasoning, research notes"]
      ];
      for (const row of rows) {
        const tr = tbody.createEl("tr");
        for (const cell of row) {
          tr.createEl("td", { text: cell });
        }
      }
      this.renderSetupGuide(containerEl, "Ollama setup guide", [
        { title: "1. Install Ollama", text: "Download and install from ollama.com. Available for Windows, macOS, and Linux." },
        { title: "2. Start the server", text: "Open a terminal and run: ollama serve. This starts the API server on localhost:11434. Keep the terminal open while using the plugin." },
        { title: "3. Pull a model", text: "In a separate terminal, pull the model you want to use:" },
        { title: "", text: "ollama pull qwen3.5:latest" },
        { title: "", text: "ollama pull gemma4:e4b" },
        { title: "", text: "ollama pull gpt-oss:20b" },
        { title: "4. Verify", text: "Run: ollama list to confirm your models are downloaded. Then select a model above and generate a note to test." }
      ]);
      new import_obsidian7.Setting(containerEl).setName("Platform notes").setHeading();
      const platformEl = containerEl.createDiv({ cls: "ch-guide" });
      const platforms = [
        {
          name: "Windows",
          notes: [
            "Install the .exe from ollama.com/download/windows.",
            "Ollama runs as a background service after installation. Check the system tray for the Ollama icon.",
            "If the tray icon is present, the server is already running. Skip 'ollama serve' and go straight to pulling models.",
            "Models are stored in C:\\Users\\<you>\\.ollama\\models."
          ]
        },
        {
          name: "macOS",
          notes: [
            "Install via the .dmg from ollama.com/download/mac, or with Homebrew: brew install ollama.",
            "The Ollama app runs as a menu bar icon and starts the server automatically.",
            "If you installed via Homebrew, start the server manually: ollama serve.",
            "Models are stored in ~/.ollama/models.",
            "Apple Silicon (M1/M2/M3/M4) Macs run models significantly faster than Intel Macs thanks to the unified GPU memory."
          ]
        },
        {
          name: "Linux",
          notes: [
            "Install with the one-liner: curl -fsSL https://ollama.com/install.sh | sh",
            "This installs Ollama and sets up a systemd service that starts automatically.",
            "Check the service status: systemctl status ollama.",
            "If not using systemd, start manually: ollama serve.",
            "Models are stored in ~/.ollama/models (or /usr/share/ollama/.ollama/models if running as the ollama user).",
            "For GPU acceleration, install NVIDIA CUDA drivers or AMD ROCm. Ollama auto-detects available GPUs."
          ]
        }
      ];
      for (const platform of platforms) {
        const pEl = platformEl.createDiv({ cls: "ch-platform-card" });
        pEl.createEl("strong", { cls: "ch-strong-block ch-gap-below", text: platform.name });
        const ul = pEl.createEl("ul");
        for (const note of platform.notes) {
          ul.createEl("li", { text: note });
        }
      }
    }
    new import_obsidian7.Setting(containerEl).setName("Enable logging").setDesc("Log CLI invocations, errors, and timing to an in-memory buffer. View with the 'View logs' command.").addToggle(
      (toggle) => toggle.setValue(this.plugin.settings.enableLogging).onChange(async (value) => {
        this.plugin.settings.enableLogging = value;
        logger.setEnabled(value);
        await this.plugin.saveSettings();
      })
    );
    new import_obsidian7.Setting(containerEl).setName("Preferred mode").setDesc("Pre-selected mode when no previous mode has been used. Leave empty to start with no selection. The modal remembers your last used mode automatically.").addDropdown((dropdown) => {
      dropdown.addOption("", "(None)");
      const standardModes = this.plugin.getStandardModes();
      for (const mode of standardModes) {
        dropdown.addOption(mode.id, mode.name);
      }
      dropdown.setValue(this.plugin.settings.defaultModeId);
      dropdown.onChange(async (value) => {
        this.plugin.settings.defaultModeId = value;
        await this.plugin.saveSettings();
      });
    });
    new import_obsidian7.Setting(containerEl).setName("Built-in note modes").setHeading();
    containerEl.createEl("p", {
      cls: "ch-muted-note",
      text: modes_default.map((m) => m.name).join(", ")
    });
    new import_obsidian7.Setting(containerEl).setName("Inline actions").setHeading();
    containerEl.createEl("p", {
      cls: "ch-muted-note",
      text: INLINE_ACTIONS.map((a) => a.name).join(", ")
    });
    new import_obsidian7.Setting(containerEl).setName("Custom modes").setHeading();
    containerEl.createEl("p", {
      text: "Add your own modes. Each mode needs an ID, name, description, and prompt template. Use {selection} and {context} as placeholders in your prompt.",
      cls: "setting-item-description"
    });
    for (let i = 0; i < this.plugin.settings.customModes.length; i++) {
      const mode = this.plugin.settings.customModes[i];
      const modeContainer = containerEl.createDiv({ cls: "ch-custom-mode" });
      new import_obsidian7.Setting(modeContainer).setName("Mode name").addText(
        (text) => text.setValue(mode.name).onChange(async (value) => {
          mode.name = value;
          mode.id = value.toLowerCase().replace(/\s+/g, "-").replace(/[^a-z0-9-]/g, "");
          await this.plugin.saveSettings();
        })
      );
      new import_obsidian7.Setting(modeContainer).setName("Description").addText(
        (text) => text.setValue(mode.description).onChange(async (value) => {
          mode.description = value;
          await this.plugin.saveSettings();
        })
      );
      new import_obsidian7.Setting(modeContainer).setName("Prompt").addTextArea((text) => {
        text.setValue(mode.prompt).onChange(async (value) => {
          mode.prompt = value;
          await this.plugin.saveSettings();
        });
        text.inputEl.rows = 6;
        text.inputEl.addClass("ch-full-width-input");
      });
      new import_obsidian7.Setting(modeContainer).addButton(
        (btn) => btn.setButtonText("Remove").setWarning().onClick(async () => {
          this.plugin.settings.customModes.splice(i, 1);
          await this.plugin.saveSettings();
          this.display();
        })
      );
    }
    new import_obsidian7.Setting(containerEl).addButton(
      (btn) => btn.setButtonText("Add custom mode").onClick(async () => {
        this.plugin.settings.customModes.push({
          id: "custom-" + Date.now(),
          name: "New Mode",
          icon: "pencil",
          description: "My custom mode",
          prompt: 'The user selected "{selection}" while reading:\n\n---\n{context}\n---\n\nWrite detailed notes about "{selection}".'
        });
        await this.plugin.saveSettings();
        this.display();
      })
    );
  }
};

// src/main.ts
var ClaudeExplainerPlugin = class extends import_obsidian8.Plugin {
  constructor() {
    super(...arguments);
    this.settings = DEFAULT_SETTINGS;
    this.queue = [];
    this.failedItems = [];
    this.completedItems = [];
    this.processingItem = null;
    this.isProcessing = false;
    this.statusBarEl = null;
    this.streamData = { currentOutput: "", startTime: 0 };
  }
  getAllModes() {
    return [...modes_default, ...this.settings.customModes];
  }
  getFullNoteModes() {
    return this.getAllModes().filter((m) => m.fullNote);
  }
  getStandardModes() {
    return this.getAllModes().filter((m) => !m.fullNote);
  }
  async onload() {
    await this.loadSettings();
    this.statusBarEl = this.addStatusBarItem();
    this.updateStatusBar();
    this.showFirstRunNotice();
    this.addCommand({
      id: "explain-selection",
      name: "Explain selection with AI",
      editorCallback: (editor, view) => {
        const selection = editor.getSelection().trim();
        if (!selection) {
          new import_obsidian8.Notice("Select some text first.");
          return;
        }
        const from = editor.getCursor("from");
        const to = editor.getCursor("to");
        new NoteCreatorModal(this.app, this.getStandardModes(), selection, this.settings.lastModeId || this.settings.defaultModeId, (configs) => {
          var _a, _b, _c;
          const linkReplacement = this.buildLinkReplacement(editor, from, to, selection, configs, (_c = (_b = (_a = view.file) == null ? void 0 : _a.parent) == null ? void 0 : _b.path) != null ? _c : "");
          for (const config of configs) {
            void this.enqueueNote(editor, view, selection, config, linkReplacement);
          }
        }, (modeId) => {
          void this.saveLastMode(modeId);
        }).open();
      }
    });
    this.addCommand({
      id: "create-sub-note",
      name: "Create sub-note from selection with AI",
      editorCallback: (editor, view) => {
        const selection = editor.getSelection().trim();
        if (!selection) {
          new import_obsidian8.Notice("Select some text first.");
          return;
        }
        this.openSubNoteCreator(editor, view, selection, editor.getCursor("from"), editor.getCursor("to"));
      }
    });
    this.addCommand({
      id: "enhance-selection",
      name: "Enhance selection with AI",
      editorCallback: (editor, view) => {
        const selection = editor.getSelection().trim();
        if (!selection) {
          new import_obsidian8.Notice("Select some text first.");
          return;
        }
        const from = editor.getCursor("from");
        const to = editor.getCursor("to");
        new InlineActionModal(this.app, INLINE_ACTIONS, selection, (config) => {
          this.enqueueInlineAction(editor, selection, config, from, to);
        }).open();
      }
    });
    for (const action of INLINE_ACTIONS) {
      this.addCommand({
        id: `inline-${action.id}`,
        name: `${action.name} selection with AI`,
        editorCallback: (editor, view) => {
          const selection = editor.getSelection().trim();
          if (!selection) {
            new import_obsidian8.Notice("Select some text first.");
            return;
          }
          const from = editor.getCursor("from");
          const to = editor.getCursor("to");
          new InlineActionModal(this.app, INLINE_ACTIONS, selection, (config) => {
            this.enqueueInlineAction(editor, selection, config, from, to);
          }, action).open();
        }
      });
    }
    this.addCommand({
      id: "view-queue",
      name: "View note generation queue",
      callback: () => {
        new QueueStatusModal(this.app, this).open();
      }
    });
    this.addCommand({
      id: "analyze-note",
      name: "Analyze current note with AI",
      editorCallback: (editor, view) => {
        const file = view.file;
        if (!file) {
          new import_obsidian8.Notice("No active file.");
          return;
        }
        const fullNoteModes = this.getFullNoteModes();
        if (fullNoteModes.length === 0) {
          new import_obsidian8.Notice("No full-note analysis modes available.");
          return;
        }
        new FullNoteActionModal(this.app, fullNoteModes, file.basename, (mode, extraInstructions) => {
          void this.enqueueAppend(editor, file, mode, extraInstructions);
        }).open();
      }
    });
    this.addCommand({
      id: "generate-topic",
      name: "Generate notes from topic with AI",
      callback: () => {
        var _a, _b;
        const activeFile = this.app.workspace.getActiveFile();
        const folderPath = (_b = (_a = activeFile == null ? void 0 : activeFile.parent) == null ? void 0 : _a.path) != null ? _b : "";
        const standardModes = this.getStandardModes();
        new TopicGeneratorModal(this.app, standardModes, folderPath, (topic, mode, isMulti, extraInstructions) => {
          void this.generateTopicNotes(topic, mode, isMulti, folderPath, extraInstructions);
        }).open();
      }
    });
    this.addCommand({
      id: "fill-empty-note",
      name: "Fill empty note with AI",
      editorCallback: async (editor, view) => {
        const file = view.file;
        if (!file) {
          new import_obsidian8.Notice("No active file.");
          return;
        }
        if (!isNoteEffectivelyEmpty(editor.getValue())) {
          new import_obsidian8.Notice("This note already has content. Use this command on an empty note.");
          return;
        }
        const { context, sources } = await this.getBacklinkContext(file);
        const standardModes = this.getStandardModes();
        new FillNoteModal(
          this.app,
          standardModes,
          file.basename,
          context,
          sources,
          this.settings.lastModeId || this.settings.defaultModeId,
          (mode, ctx, extraInstructions) => {
            void this.enqueueFillNote(file, mode, ctx, extraInstructions);
          },
          (modeId) => {
            void this.saveLastMode(modeId);
          }
        ).open();
      }
    });
    this.addCommand({
      id: "scale-calculator",
      name: "Insert scale estimation table",
      editorCallback: (editor) => {
        new ScaleCalculatorModal(this.app, (md) => {
          const cursor = editor.getCursor();
          editor.replaceRange("\n" + md + "\n", cursor);
        }).open();
      }
    });
    this.addCommand({
      id: "scaffold-sd-workspace",
      name: "Scaffold system design workspace",
      callback: async () => {
        new ScaffoldWorkspaceModal(this.app, async (name, sections) => {
          await this.scaffoldWorkspace(name, sections);
        }).open();
      }
    });
    this.registerEvent(
      this.app.workspace.on("editor-menu", (menu, editor) => {
        const view = this.app.workspace.getActiveViewOfType(import_obsidian8.MarkdownView);
        if (!view)
          return;
        const selection = editor.getSelection().trim();
        const from = editor.getCursor("from");
        const to = editor.getCursor("to");
        if (selection) {
          const selLabel = selection.slice(0, 30) + (selection.length > 30 ? "..." : "");
          menu.addItem((item) => {
            item.setTitle(`Create note from "${selLabel}"`).setIcon("book-open").onClick(() => {
              new NoteCreatorModal(this.app, this.getStandardModes(), selection, this.settings.lastModeId || this.settings.defaultModeId, (configs) => {
                var _a, _b, _c;
                const linkReplacement = this.buildLinkReplacement(editor, from, to, selection, configs, (_c = (_b = (_a = view.file) == null ? void 0 : _a.parent) == null ? void 0 : _b.path) != null ? _c : "");
                for (const config of configs) {
                  void this.enqueueNote(editor, view, selection, config, linkReplacement);
                }
              }, (modeId) => {
                void this.saveLastMode(modeId);
              }).open();
            });
          });
          menu.addItem((item) => {
            item.setTitle(`Create sub-note from "${selLabel}"`).setIcon("folder-tree").onClick(() => {
              this.openSubNoteCreator(editor, view, selection, from, to);
            });
          });
          menu.addItem((item) => {
            item.setTitle(`Enhance "${selLabel}" with AI`).setIcon("wand").onClick(() => {
              new InlineActionModal(this.app, INLINE_ACTIONS, selection, (config) => {
                this.enqueueInlineAction(editor, selection, config, from, to);
              }).open();
            });
          });
        }
        if (selection) {
          menu.addItem((item) => {
            const sub = item.setTitle("AI actions").setIcon("sparkles").setSubmenu();
            for (const action of INLINE_ACTIONS) {
              sub.addItem((subItem) => {
                subItem.setTitle(action.name).setIcon(action.icon).onClick(() => {
                  new InlineActionModal(this.app, INLINE_ACTIONS, selection, (config) => {
                    this.enqueueInlineAction(editor, selection, config, from, to);
                  }, action).open();
                });
              });
            }
          });
        }
        const file = view.file;
        if (file) {
          const fullNoteModes = this.getFullNoteModes();
          if (fullNoteModes.length > 0) {
            menu.addItem((item) => {
              item.setTitle("Analyze this note with AI").setIcon("brain").onClick(() => {
                new FullNoteActionModal(this.app, fullNoteModes, file.basename, (mode, extraInstructions) => {
                  void this.enqueueAppend(editor, file, mode, extraInstructions);
                }).open();
              });
            });
          }
          if (isNoteEffectivelyEmpty(editor.getValue())) {
            menu.addItem((item) => {
              item.setTitle("Fill this empty note with AI").setIcon("file-plus").onClick(async () => {
                const { context, sources } = await this.getBacklinkContext(file);
                const standardModes = this.getStandardModes();
                new FillNoteModal(
                  this.app,
                  standardModes,
                  file.basename,
                  context,
                  sources,
                  this.settings.lastModeId || this.settings.defaultModeId,
                  (mode, ctx, extraInstructions) => {
                    void this.enqueueFillNote(file, mode, ctx, extraInstructions);
                  },
                  (modeId) => {
                    void this.saveLastMode(modeId);
                  }
                ).open();
              });
            });
          }
        }
      })
    );
    this.registerEvent(
      this.app.workspace.on("file-menu", (menu, file) => {
        if (!(file instanceof import_obsidian8.TFolder))
          return;
        menu.addItem((item) => {
          item.setTitle("Generate knowledge notes with AI").setIcon("sparkles").onClick(() => {
            const standardModes = this.getStandardModes();
            new FolderGeneratorModal(
              this.app,
              standardModes,
              file.path,
              file.name,
              (scenario, mode, extraInstructions) => {
                void this.generateKnowledgeNotes(scenario, mode, file.path, extraInstructions);
              }
            ).open();
          });
        });
        menu.addItem((item) => {
          item.setTitle("Add topic to folder with AI").setIcon("file-plus").onClick(() => {
            const standardModes = this.getStandardModes();
            new AddTopicModal(
              this.app,
              standardModes,
              file.path,
              file.name,
              (topic, mode, extraInstructions) => {
                void this.addTopicToFolder(topic, mode, file, extraInstructions);
              }
            ).open();
          });
        });
        menu.addItem((item) => {
          item.setTitle("Expand folder with more notes").setIcon("layers").onClick(() => {
            const standardModes = this.getStandardModes();
            const existingNotes = file.children.filter((f) => f instanceof import_obsidian8.TFile && f.extension === "md").map((f) => f.basename);
            new ExpandFolderModal(
              this.app,
              standardModes,
              file.path,
              file.name,
              existingNotes,
              (direction, mode, extraInstructions) => {
                void this.expandFolderNotes(direction, mode, file, extraInstructions);
              }
            ).open();
          });
        });
        menu.addItem((item) => {
          item.setTitle("Analyze folder with AI").setIcon("search").onClick(() => {
            const analysisModes = this.getAllModes().filter((m) => m.analyzesExisting);
            new FolderAnalysisModal(
              this.app,
              analysisModes,
              file.path,
              file.name,
              (mode, extraContent) => {
                void this.analyzeFolderNotes(file, mode, extraContent);
              }
            ).open();
          });
        });
      })
    );
    this.registerEvent(
      this.app.workspace.on("file-menu", (menu, file) => {
        if (!(file instanceof import_obsidian8.TFile) || file.extension !== "md")
          return;
        menu.addItem((item) => {
          item.setTitle("Analyze note with AI").setIcon("search").onClick(() => {
            const analysisModes = this.getAllModes().filter((m) => m.analyzesExisting);
            new NoteAnalysisModal(
              this.app,
              analysisModes,
              file,
              (mode, extraContent) => {
                void this.analyzeExistingNote(file, mode, extraContent);
              }
            ).open();
          });
        });
      })
    );
    this.addCommand({
      id: "generate-knowledge-notes",
      name: "Generate knowledge notes in folder with AI",
      callback: () => {
        var _a, _b, _c, _d;
        const activeFile = this.app.workspace.getActiveFile();
        const folderPath = (_b = (_a = activeFile == null ? void 0 : activeFile.parent) == null ? void 0 : _a.path) != null ? _b : "";
        const folderName = (_d = (_c = activeFile == null ? void 0 : activeFile.parent) == null ? void 0 : _c.name) != null ? _d : "vault root";
        const standardModes = this.getStandardModes();
        new FolderGeneratorModal(
          this.app,
          standardModes,
          folderPath,
          folderName,
          (scenario, mode, extraInstructions) => {
            void this.generateKnowledgeNotes(scenario, mode, folderPath, extraInstructions);
          }
        ).open();
      }
    });
    this.addCommand({
      id: "view-logs",
      name: "View logs",
      callback: () => {
        new LogViewerModal(this.app).open();
      }
    });
    this.addSettingTab(new ClaudeExplainerSettingTab(this.app, this));
    logger.info("Plugin loaded");
  }
  async loadSettings() {
    this.settings = Object.assign({}, DEFAULT_SETTINGS, await this.loadData());
    logger.setEnabled(this.settings.enableLogging);
  }
  async saveSettings() {
    await this.saveData(this.settings);
  }
  getQueue() {
    return [...this.queue];
  }
  getProcessingItem() {
    return this.processingItem;
  }
  getStreamData() {
    return this.streamData;
  }
  async saveLastMode(modeId) {
    this.settings.lastModeId = modeId;
    await this.saveSettings();
  }
  async saveLastSubfolder(subfolder) {
    this.settings.lastSubfolder = subfolder;
    await this.saveSettings();
  }
  removeFromQueue(index) {
    this.queue.splice(index, 1);
    this.updateStatusBar();
  }
  clearQueue() {
    this.queue = [];
    this.updateStatusBar();
    new import_obsidian8.Notice("Queue cleared.");
  }
  getFailedItems() {
    return [...this.failedItems];
  }
  getCompletedItems() {
    return [...this.completedItems];
  }
  retryFailed() {
    if (this.failedItems.length === 0) {
      new import_obsidian8.Notice("No failed items to retry.");
      return;
    }
    const count = this.failedItems.length;
    for (const f of this.failedItems) {
      this.queue.push(f.item);
    }
    this.failedItems = [];
    this.updateStatusBar();
    new import_obsidian8.Notice(`Re-queued ${count} failed item(s).`);
    if (!this.isProcessing) {
      void this.processQueue();
    }
  }
  retryOne(index) {
    if (index < 0 || index >= this.failedItems.length)
      return;
    const [removed] = this.failedItems.splice(index, 1);
    this.queue.push(removed.item);
    this.updateStatusBar();
    new import_obsidian8.Notice(`Re-queued "${this.getItemLabel(removed.item)}".`);
    if (!this.isProcessing) {
      void this.processQueue();
    }
  }
  dismissFailed(index) {
    if (index < 0 || index >= this.failedItems.length)
      return;
    this.failedItems.splice(index, 1);
    this.updateStatusBar();
  }
  clearFailed() {
    this.failedItems = [];
    this.updateStatusBar();
  }
  getItemLabel(item) {
    if (item.type === "note" || item.type === "topic-note")
      return item.noteName;
    if (item.type === "append")
      return `${item.mode.name} on ${item.fileName}`;
    return item.action.name;
  }
  updateStatusBar() {
    if (!this.statusBarEl)
      return;
    const total = this.queue.length + (this.isProcessing ? 1 : 0);
    if (total === 0) {
      this.statusBarEl.setText("");
    } else if (this.isProcessing && this.processingItem) {
      let label;
      if (this.processingItem.type === "note" || this.processingItem.type === "topic-note") {
        label = this.processingItem.noteName;
      } else if (this.processingItem.type === "append") {
        label = `[${this.processingItem.mode.name}] ${this.processingItem.fileName}`;
      } else {
        label = `[${this.processingItem.action.name}]`;
      }
      const failSuffix = this.failedItems.length > 0 ? `, ${this.failedItems.length} failed` : "";
      const ai = PROVIDER_LABELS[this.settings.aiProvider];
      this.statusBarEl.setText(`${ai}: generating "${label}" (${this.queue.length} queued${failSuffix})`);
    } else if (this.failedItems.length > 0) {
      const ai = PROVIDER_LABELS[this.settings.aiProvider];
      this.statusBarEl.setText(`${ai}: ${this.failedItems.length} failed (click to retry)`);
    } else {
      const ai = PROVIDER_LABELS[this.settings.aiProvider];
      this.statusBarEl.setText(`${ai}: ${this.queue.length} in queue`);
    }
  }
  showFirstRunNotice() {
    if (this.settings.setupNoticeShown)
      return;
    this.settings.setupNoticeShown = true;
    void this.saveSettings();
    const notice = new import_obsidian8.Notice(
      "Second Brain Builder needs an AI backend before first use: the Claude Code CLI (default) or a free local Ollama server. Click here to open settings and follow the setup guide.",
      0
    );
    notice.messageEl.addEventListener("click", () => {
      notice.hide();
      this.openSettingsTab();
    });
  }
  openSettingsTab() {
    const appWithSettings = this.app;
    appWithSettings.setting.open();
    appWithSettings.setting.openTabById(this.manifest.id);
  }
  openSubNoteCreator(editor, view, selection, from, to) {
    var _a, _b, _c;
    new NoteCreatorModal(
      this.app,
      this.getStandardModes(),
      selection,
      this.settings.lastModeId || this.settings.defaultModeId,
      (configs) => {
        var _a2, _b2, _c2, _d, _e, _f;
        const parentName = (_b2 = (_a2 = view.file) == null ? void 0 : _a2.basename) != null ? _b2 : "";
        const titles = configs.map((c) => c.title);
        const contextOverride = this.buildSelectionContext(editor, from, selection);
        for (const config of configs) {
          config.seriesInstructions = this.buildSubNoteSeriesInstructions(parentName, config.title, titles, selection);
          config.contextOverride = contextOverride;
        }
        const topicsMatchSelection = configs.every(
          (c) => selection.toLowerCase().includes(c.title.toLowerCase())
        );
        const linkReplacement = this.buildLinkReplacement(editor, from, to, selection, configs, (_e = (_d = (_c2 = view.file) == null ? void 0 : _c2.parent) == null ? void 0 : _d.path) != null ? _e : "", !topicsMatchSelection);
        for (const config of configs) {
          void this.enqueueNote(editor, view, config.title, config, linkReplacement);
        }
        if ((_f = configs[0]) == null ? void 0 : _f.subfolder) {
          void this.saveLastSubfolder(configs[0].subfolder);
        }
      },
      (modeId) => {
        void this.saveLastMode(modeId);
      },
      this.settings.lastSubfolder || DEFAULT_SETTINGS.lastSubfolder,
      (_c = (_b = (_a = view.file) == null ? void 0 : _a.parent) == null ? void 0 : _b.path) != null ? _c : ""
    ).open();
  }
  // The note context sent with each sub-note: a window of the parent note
  // centered on the selection, so the selected passage is always included
  // even in long notes.
  buildSelectionContext(editor, from, selection) {
    const content = editor.getValue();
    if (content.length <= 2e3)
      return content;
    const selStart = editor.posToOffset(from);
    const windowStart = Math.max(0, selStart - Math.floor((2e3 - selection.length) / 2));
    const windowEnd = Math.min(content.length, windowStart + 2e3 + selection.length);
    return (windowStart > 0 ? "...(earlier content omitted)\n" : "") + content.slice(windowStart, windowEnd) + (windowEnd < content.length ? "\n...(later content omitted)" : "");
  }
  buildSubNoteSeriesInstructions(parentName, title, allTitles, selection) {
    const siblings = allTitles.filter((t) => t !== title);
    const lines = [
      `This note is part of a linked sub-note series generated from the note "${parentName}".`,
      `The user selected this passage in that note: "${selection.replace(/"/g, '\\"')}". Stay grounded in it: the note should explain its topic in a way that illuminates this passage.`,
      `Link back to the parent note as [[${parentName}]] in a Related Concepts or similar section.`
    ];
    if (siblings.length > 0) {
      lines.push(`Sibling notes being generated in the same series: ${siblings.map((t) => `[[${t}]]`).join(", ")}. Cross-link them with [[wiki-links]] wherever they are relevant to the content.`);
    }
    lines.push("Also reuse [[wiki-links]] that already appear in the provided context where relevant, so this note connects to the existing note series.");
    return lines.join(" ");
  }
  buildLinkReplacement(editor, from, to, selection, configs, folderPath, keepSelectionText = false) {
    const links = configs.map((c) => {
      if (!c.subfolder)
        return `[[${c.title}]]`;
      const dir = folderPath ? `${folderPath}/${c.subfolder}` : c.subfolder;
      return `[[${dir}/${c.title}|${c.title}]]`;
    });
    return {
      editor,
      from,
      to,
      selection,
      linksText: keepSelectionText ? `${selection} (${links.join(", ")})` : links.join(" | "),
      applied: false
    };
  }
  applyLinkReplacement(replacement) {
    if (!replacement || replacement.applied)
      return;
    replacement.applied = true;
    try {
      if (replacement.editor.getRange(replacement.from, replacement.to) !== replacement.selection) {
        logger.info("Selection changed since queuing; skipping wiki-link insertion.");
        return;
      }
      replacement.editor.replaceRange(replacement.linksText, replacement.from, replacement.to);
    } catch (err) {
      logger.error(`Could not insert wiki-link: ${err instanceof Error ? err.message : String(err)}`);
    }
  }
  async enqueueNote(editor, view, selection, config, linkReplacement) {
    var _a, _b, _c;
    const file = view.file;
    if (!file) {
      new import_obsidian8.Notice("No active file.");
      return;
    }
    const folderPath = (_b = (_a = file.parent) == null ? void 0 : _a.path) != null ? _b : "";
    const noteName = config.title;
    if (!noteName) {
      new import_obsidian8.Notice("Invalid note title.");
      return;
    }
    const targetFolder = config.subfolder ? (0, import_obsidian8.normalizePath)(folderPath ? `${folderPath}/${config.subfolder}` : config.subfolder) : folderPath;
    if (config.subfolder && !this.app.vault.getAbstractFileByPath(targetFolder)) {
      await this.app.vault.createFolder(targetFolder).catch(() => {
      });
    }
    const newNotePath = (0, import_obsidian8.normalizePath)(
      targetFolder ? `${targetFolder}/${noteName}.md` : `${noteName}.md`
    );
    const existing = this.app.vault.getAbstractFileByPath(newNotePath);
    if (existing instanceof import_obsidian8.TFile) {
      const existingContent = await this.app.vault.read(existing);
      if (!isNoteEffectivelyEmpty(existingContent)) {
        this.applyLinkReplacement(linkReplacement);
        new import_obsidian8.Notice(`Note "${noteName}" already has content. Linked to it.`);
        return;
      }
    }
    const contextLines = editor.getValue();
    const contextSnippet = (_c = config.contextOverride) != null ? _c : contextLines.length > 2e3 ? contextLines.slice(0, 2e3) + "\n...(truncated)" : contextLines;
    let fullPrompt = config.mode.prompt.replace(/\{selection\}/g, selection.replace(/"/g, '\\"')).replace(/\{context\}/g, contextSnippet.replace(/"/g, '\\"')) + getOutputRules();
    if (config.seriesInstructions) {
      fullPrompt += `

${config.seriesInstructions}`;
    }
    if (config.extraInstructions) {
      fullPrompt += `

Additional instructions from the user:
${config.extraInstructions}`;
    }
    this.queue.push({
      type: "note",
      selection,
      noteName,
      newNotePath,
      fullPrompt,
      mode: config.mode,
      editor,
      linkReplacement
    });
    const pos = this.queue.length + (this.isProcessing ? 1 : 0);
    new import_obsidian8.Notice(`Queued "${noteName}" (${config.mode.name}) - position ${pos}`);
    this.updateStatusBar();
    if (!this.isProcessing) {
      void this.processQueue();
    }
  }
  enqueueInlineAction(editor, selection, config, from, to) {
    const contextLines = editor.getValue();
    const contextSnippet = contextLines.length > 2e3 ? contextLines.slice(0, 2e3) + "\n...(truncated)" : contextLines;
    let fullPrompt = config.action.prompt.replace(/\{selection\}/g, selection.replace(/"/g, '\\"')).replace(/\{context\}/g, contextSnippet.replace(/"/g, '\\"')) + INLINE_OUTPUT_RULES;
    if (config.extraInstructions) {
      fullPrompt += `

Additional instructions from the user:
${config.extraInstructions}`;
    }
    this.queue.push({
      type: "inline",
      selection,
      fullPrompt,
      action: config.action,
      editor,
      from,
      to,
      insertionMode: config.insertionMode
    });
    const pos = this.queue.length + (this.isProcessing ? 1 : 0);
    new import_obsidian8.Notice(`Queued "${config.action.name}" (${config.insertionMode}) - position ${pos}`);
    this.updateStatusBar();
    if (!this.isProcessing) {
      void this.processQueue();
    }
  }
  async getBacklinkContext(file) {
    const sources = [];
    const contextParts = [];
    const targetPath = file.path;
    const resolvedLinks = this.app.metadataCache.resolvedLinks;
    for (const sourcePath of Object.keys(resolvedLinks)) {
      const links = resolvedLinks[sourcePath];
      if (links && links[targetPath]) {
        const sourceFile = this.app.vault.getAbstractFileByPath(sourcePath);
        if (sourceFile instanceof import_obsidian8.TFile) {
          sources.push(sourceFile.basename);
          const content = await this.app.vault.read(sourceFile);
          const snippet = content.length > 1500 ? content.slice(0, 1500) + "\n...(truncated)" : content;
          contextParts.push(`From [[${sourceFile.basename}]]:
${snippet}`);
        }
      }
      if (sources.length >= 5)
        break;
    }
    const context = contextParts.join("\n\n---\n\n");
    return { context, sources };
  }
  async enqueueFillNote(file, mode, context, extraInstructions) {
    const topic = file.basename;
    const contextBlock = context.length > 0 ? context : "No existing context provided. Generate a comprehensive note on this topic from scratch.";
    let fullPrompt = mode.prompt.replace(/\{selection\}/g, topic.replace(/"/g, '\\"')).replace(/\{context\}/g, contextBlock.replace(/"/g, '\\"')) + getOutputRules();
    if (extraInstructions) {
      fullPrompt += `

Additional instructions from the user:
${extraInstructions}`;
    }
    this.queue.push({
      type: "topic-note",
      noteName: topic,
      newNotePath: file.path,
      fullPrompt,
      mode
    });
    const pos = this.queue.length + (this.isProcessing ? 1 : 0);
    new import_obsidian8.Notice(`Queued "${topic}" (${mode.name}) - position ${pos}`);
    this.updateStatusBar();
    if (!this.isProcessing) {
      void this.processQueue();
    }
  }
  async enqueueAppend(editor, file, mode, extraInstructions) {
    const fullContent = editor.getValue();
    let fullPrompt = mode.prompt.replace(/\{selection\}/g, "").replace(/\{context\}/g, fullContent.replace(/"/g, '\\"')) + APPEND_OUTPUT_RULES;
    if (extraInstructions) {
      fullPrompt += `

Additional instructions from the user:
${extraInstructions}`;
    }
    this.queue.push({
      type: "append",
      filePath: file.path,
      fileName: file.basename,
      fullPrompt,
      mode
    });
    const pos = this.queue.length + (this.isProcessing ? 1 : 0);
    new import_obsidian8.Notice(`Queued "${mode.name}" for ${file.basename} - position ${pos}`);
    this.updateStatusBar();
    if (!this.isProcessing) {
      void this.processQueue();
    }
  }
  async scaffoldWorkspace(name, sections) {
    const basePath = (0, import_obsidian8.normalizePath)(name);
    const sectionDescriptions = {
      Systems: "Full system design notes. Use the **System Design** mode to generate notes here.",
      Patterns: "Design pattern cards. Use the **SD Concept** mode with pattern topics (Saga, CQRS, Event Sourcing).",
      Components: "Technology deep-dives. Use the **SD Concept** mode with technology topics (Redis, Kafka, Postgres).",
      Failures: "Failure mode analysis. Use the **SD Concept** mode with failure topics (Cache Stampede, Split Brain).",
      Tradeoffs: "Technology comparisons. Use the **SD Concept** or **Compare & Contrast** mode.",
      Simulations: "Request flow traces and path simulations.",
      Glossary: "Term definitions and concept maps. Use the **Glossary Builder** mode."
    };
    try {
      for (const section of sections) {
        const folderPath = (0, import_obsidian8.normalizePath)(`${basePath}/${section}`);
        const existing = this.app.vault.getAbstractFileByPath(folderPath);
        if (!existing) {
          await this.app.vault.createFolder(folderPath);
        }
      }
      const indexLines = [`# ${name}
`];
      indexLines.push("A structured workspace for system design study and architecture notes.\n");
      for (const section of sections) {
        const desc = sectionDescriptions[section] || "";
        indexLines.push(`## [[${section}]]
`);
        indexLines.push(`${desc}
`);
      }
      indexLines.push("---\n");
      indexLines.push("*Scaffold created by Second Brain Builder.*\n");
      const indexPath = (0, import_obsidian8.normalizePath)(`${basePath}/${name}.md`);
      const existingIndex = this.app.vault.getAbstractFileByPath(indexPath);
      if (!existingIndex) {
        await this.app.vault.create(indexPath, indexLines.join("\n"));
      }
      new import_obsidian8.Notice(`Workspace "${name}" created with ${sections.length} sections.`);
      const file = this.app.vault.getAbstractFileByPath(indexPath);
      if (file instanceof import_obsidian8.TFile) {
        await this.app.workspace.getLeaf().openFile(file);
      }
    } catch (err) {
      new import_obsidian8.Notice(`Error creating workspace: ${err}`);
    }
  }
  async generateTopicNotes(topic, mode, isMulti, folderPath, extraInstructions) {
    const sanitizedTopic = sanitizeFilename(topic);
    if (!isMulti) {
      const noteName = sanitizedTopic;
      const newNotePath = (0, import_obsidian8.normalizePath)(
        folderPath ? `${folderPath}/${noteName}.md` : `${noteName}.md`
      );
      const existing = this.app.vault.getAbstractFileByPath(newNotePath);
      if (existing instanceof import_obsidian8.TFile) {
        const existingContent = await this.app.vault.read(existing);
        if (!isNoteEffectivelyEmpty(existingContent)) {
          new import_obsidian8.Notice(`Note "${noteName}" already has content.`);
          return;
        }
      }
      const contextBlock = "This note is being created from scratch. There is no existing source note.";
      let fullPrompt = mode.prompt.replace(/\{selection\}/g, topic.replace(/"/g, '\\"')).replace(/\{context\}/g, contextBlock) + getOutputRules();
      if (extraInstructions) {
        fullPrompt += `

Additional instructions from the user:
${extraInstructions}`;
      }
      this.queue.push({
        type: "topic-note",
        noteName,
        newNotePath,
        fullPrompt,
        mode
      });
      const pos = this.queue.length + (this.isProcessing ? 1 : 0);
      new import_obsidian8.Notice(`Queued "${noteName}" (${mode.name}) - position ${pos}`);
      this.updateStatusBar();
      if (!this.isProcessing) {
        void this.processQueue();
      }
      return;
    }
    new import_obsidian8.Notice(`Planning notes for "${topic}"... This may take a moment.`);
    const decompositionPrompt = `You are a curriculum designer creating a structured set of study notes.

Topic: "${topic}"

Break this topic down into a logical set of 4-10 sub-topics that together provide comprehensive coverage. Each sub-topic should be a self-contained note that can link to related sub-topics.

Return ONLY valid JSON - no markdown code fences, no explanation, no text before or after. The format:
[
  {"title": "Sub-topic Title", "description": "Brief description of what this note covers", "related": ["Other Sub-topic Title"]}
]

Rules:
- Order sub-topics from foundational to advanced
- Titles should be clear, standalone note titles (3-8 words)
- "related" should reference other sub-topic titles from your list that should be cross-linked
- Do not include the main topic itself as a sub-topic - it will be the index note
${extraInstructions ? `
Additional context: ${extraInstructions}` : ""}`;
    try {
      const rawResponse = await this.runClaude(decompositionPrompt);
      let subtopics;
      try {
        const jsonStr = extractJsonArray(rawResponse);
        subtopics = JSON.parse(jsonStr);
      } catch (e) {
        new import_obsidian8.Notice("Failed to parse topic decomposition. Falling back to single note.", 8e3);
        console.error("Topic decomposition parse error. Raw response:", rawResponse);
        void this.generateTopicNotes(topic, mode, false, folderPath, extraInstructions);
        return;
      }
      if (!Array.isArray(subtopics) || subtopics.length === 0) {
        new import_obsidian8.Notice("No sub-topics generated. Falling back to single note.", 5e3);
        void this.generateTopicNotes(topic, mode, false, folderPath, extraInstructions);
        return;
      }
      const topicFolder = (0, import_obsidian8.normalizePath)(
        folderPath ? `${folderPath}/${sanitizedTopic}` : sanitizedTopic
      );
      const existingFolder = this.app.vault.getAbstractFileByPath(topicFolder);
      if (!existingFolder) {
        await this.app.vault.createFolder(topicFolder);
      }
      const subtopicList = subtopics.map((s) => `- [[${sanitizeFilename(s.title)}]]: ${s.description}`).join("\n");
      const indexPrompt = `You are creating an index/hub note for a series of study notes on "${topic}".

The series contains these notes:
${subtopicList}

Write an index note that:
1. Opens with a brief overview of the topic (2-3 paragraphs)
2. Lists all notes in the series using [[Note Title]] wiki-link format with brief descriptions
3. Suggests a recommended reading order
4. Includes a mermaid diagram showing how the notes relate to each other

IMPORTANT mermaid syntax rules: always wrap node labels and subgraph titles in double quotes. Use plain hyphens instead of em dashes. Keep mermaid syntax strictly ASCII-safe. Never use markdown list syntax inside labels (no "- item", "* item", "1. step", or "1) step"); for numbered steps in edge labels use a colon "1:" not "1." to avoid "Unsupported markdown: list" errors.

Use [[Note Title]] format for all links to sub-notes.` + getOutputRules();
      const indexPath = (0, import_obsidian8.normalizePath)(`${topicFolder}/${sanitizedTopic}.md`);
      this.queue.push({
        type: "topic-note",
        noteName: sanitizedTopic + " (index)",
        newNotePath: indexPath,
        fullPrompt: indexPrompt,
        mode
      });
      for (const sub of subtopics) {
        const subName = sanitizeFilename(sub.title);
        const subPath = (0, import_obsidian8.normalizePath)(`${topicFolder}/${subName}.md`);
        const siblingContext = `This note is part of a series on "${topic}". Other notes in this series:
${subtopicList}

This specific note should focus on: ${sub.description}`;
        let subPrompt = mode.prompt.replace(/\{selection\}/g, sub.title.replace(/"/g, '\\"')).replace(/\{context\}/g, siblingContext.replace(/"/g, '\\"')) + getOutputRules();
        if (extraInstructions) {
          subPrompt += `

Additional instructions from the user:
${extraInstructions}`;
        }
        subPrompt += `

IMPORTANT: Link to related notes in this series using [[WikiLink]] format where relevant. Related notes: ${sub.related.map((r) => `[[${sanitizeFilename(r)}]]`).join(", ")}`;
        this.queue.push({
          type: "topic-note",
          noteName: subName,
          newNotePath: subPath,
          fullPrompt: subPrompt,
          mode
        });
      }
      const total = subtopics.length + 1;
      new import_obsidian8.Notice(`Queued ${total} notes for "${topic}" (1 index + ${subtopics.length} sub-notes)`);
      this.updateStatusBar();
      if (!this.isProcessing) {
        void this.processQueue();
      }
    } catch (err) {
      const msg = err instanceof Error ? err.message : String(err);
      new import_obsidian8.Notice(`Failed to plan topic notes: ${msg}`, 8e3);
      console.error("Topic planning error:", err);
    }
  }
  async generateKnowledgeNotes(scenario, mode, folderPath, extraInstructions) {
    new import_obsidian8.Notice(`Planning knowledge notes... This may take a moment.`);
    logger.info(`Knowledge notes: scenario="${scenario.slice(0, 80)}...", mode=${mode.id}, folder=${folderPath}`);
    const decompositionGuide = mode.folderDecompositionGuide ? `

DOMAIN-SPECIFIC GUIDANCE for "${mode.name}" mode:
${mode.folderDecompositionGuide}` : "";
    const decompositionPrompt = `You are an expert curriculum designer creating a structured set of deep-dive study notes.

The user wants to learn about the following scenario/topic:

"${scenario}"

The notes will be written in the "${mode.name}" style (${mode.description}).${decompositionGuide}

Break this into a logical set of 8-15 concept notes that together provide comprehensive, book-level coverage. Each note should deep-dive into ONE concept and be self-contained while linking to related notes.

Return ONLY valid JSON - no markdown code fences, no explanation, no text before or after. The format:
[
  {"title": "Note Title", "scope": "2-3 sentence description of what this specific note covers in depth", "related": ["Other Note Title"], "order": 1}
]

Rules:
- Order notes from foundational to advanced (use the "order" field, starting at 1)
- Titles should be clear, standalone note titles (3-8 words)
- "scope" should describe what the note covers in enough detail to guide a thorough write-up
- "related" should reference other note titles from your list for cross-linking
- Cover the topic comprehensively - imagine condensing the best chapters from major reference books into these notes
- Include both conceptual notes AND practical/applied notes (e.g. a request flow trace, a worked example, a comparison)
- Do not include an index or overview note - that will be generated separately
${extraInstructions ? `
Additional context from the user: ${extraInstructions}` : ""}`;
    try {
      const rawResponse = await this.runClaude(decompositionPrompt);
      let subtopics;
      try {
        const jsonStr = extractJsonArray(rawResponse);
        subtopics = JSON.parse(jsonStr);
      } catch (e) {
        new import_obsidian8.Notice("Failed to parse note plan. Check logs for details.", 8e3);
        logger.error("Knowledge notes parse error. Raw response: " + rawResponse.slice(0, 500));
        return;
      }
      if (!Array.isArray(subtopics) || subtopics.length === 0) {
        new import_obsidian8.Notice("No notes were planned. Try rephrasing the scenario.", 5e3);
        return;
      }
      subtopics.sort((a, b) => (a.order || 0) - (b.order || 0));
      const existingFolder = this.app.vault.getAbstractFileByPath(folderPath);
      if (!existingFolder) {
        await this.app.vault.createFolder(folderPath);
      }
      const subtopicList = subtopics.map((s) => `- [[${sanitizeFilename(s.title)}]]: ${s.scope}`).join("\n");
      const scenarioForIndex = scenario.length > 4e3 ? scenario.slice(0, 4e3) + "\n...(truncated)" : scenario;
      const indexName = folderPath.split("/").pop() || "Index";
      const indexPrompt = `You are creating an index/hub note for a series of deep-dive study notes.

The user's original scenario/prompt was:
"${scenarioForIndex}"

The series contains these notes (in recommended reading order):
${subtopicList}

Write an index note that:
1. Opens with a concise overview of the scenario and what the reader will learn (2-3 short paragraphs)
2. Lists all notes in the series using [[Note Title]] wiki-link format, grouped logically, with one-line descriptions
3. Suggests a recommended reading order with brief rationale
4. Includes a mermaid diagram showing how the notes relate to each other (use subgraphs to group related notes)

IMPORTANT mermaid syntax rules: always wrap node labels and subgraph titles in double quotes. Use plain hyphens instead of em dashes. Keep mermaid syntax strictly ASCII-safe. Never use markdown list syntax inside labels (no "- item", "* item", "1. step", or "1) step"); for numbered steps in edge labels use a colon "1:" not "1." to avoid "Unsupported markdown: list" errors.

Use [[Note Title]] format for all links to sub-notes.` + getOutputRules();
      const indexPath = (0, import_obsidian8.normalizePath)(`${folderPath}/${sanitizeFilename(indexName)}.md`);
      const existingIndex = this.app.vault.getAbstractFileByPath(indexPath);
      if (!existingIndex) {
        this.queue.push({
          type: "topic-note",
          noteName: sanitizeFilename(indexName) + " (index)",
          newNotePath: indexPath,
          fullPrompt: indexPrompt,
          mode
        });
      }
      const scenarioSnippet = scenario.length > 2e3 ? scenario.slice(0, 2e3) + "\n...(truncated)" : scenario;
      for (const sub of subtopics) {
        const subName = sanitizeFilename(sub.title);
        const subPath = (0, import_obsidian8.normalizePath)(`${folderPath}/${subName}.md`);
        const existingSub = this.app.vault.getAbstractFileByPath(subPath);
        if (existingSub instanceof import_obsidian8.TFile) {
          const content = await this.app.vault.read(existingSub);
          if (!isNoteEffectivelyEmpty(content)) {
            logger.info(`Skipping "${subName}" - already has content`);
            continue;
          }
        }
        const siblingContext = `This note is part of a deep-dive series. The user's scenario/prompt was:
"${scenarioSnippet}"

Other notes in this series:
${subtopicList}

This specific note should focus on: ${sub.scope}

Ground your explanations in the context of the user's scenario where relevant. Make connections to the other notes in the series.`;
        let subPrompt = mode.prompt.replace(/\{selection\}/g, sub.title.replace(/"/g, '\\"')).replace(/\{context\}/g, siblingContext.replace(/"/g, '\\"')) + getOutputRules();
        if (extraInstructions) {
          subPrompt += `

Additional instructions from the user:
${extraInstructions}`;
        }
        const relatedLinks = sub.related.map((r) => `[[${sanitizeFilename(r)}]]`).join(", ");
        subPrompt += `

IMPORTANT: Link to related notes in this series using [[WikiLink]] format where relevant. Related notes: ${relatedLinks}`;
        this.queue.push({
          type: "topic-note",
          noteName: subName,
          newNotePath: subPath,
          fullPrompt: subPrompt,
          mode
        });
      }
      const total = this.queue.length;
      new import_obsidian8.Notice(`Queued ${total} notes for knowledge series (${subtopics.length} concepts + index)`);
      this.updateStatusBar();
      if (!this.isProcessing) {
        void this.processQueue();
      }
    } catch (err) {
      const msg = err instanceof Error ? err.message : String(err);
      new import_obsidian8.Notice(`Failed to plan knowledge notes: ${msg}`, 8e3);
      logger.error("Knowledge notes planning error: " + msg);
    }
  }
  async addTopicToFolder(topic, mode, folder, extraInstructions) {
    const files = folder.children.filter(
      (f) => f instanceof import_obsidian8.TFile && f.extension === "md"
    );
    let siblingContext = "";
    const siblingList = [];
    for (const f of files) {
      const content = await this.app.vault.read(f);
      if (!isNoteEffectivelyEmpty(content)) {
        siblingList.push(`- [[${f.basename}]]`);
        const snippet = content.length > 2e3 ? content.slice(0, 2e3) + "\n..." : content;
        siblingContext += `

=== ${f.basename} ===
${snippet}`;
      }
    }
    const contextSnippet = siblingContext.length > 12e3 ? siblingContext.slice(0, 12e3) + "\n...(truncated)" : siblingContext;
    const noteName = sanitizeFilename(topic.split("\n")[0].trim());
    const notePath = (0, import_obsidian8.normalizePath)(`${folder.path}/${noteName}.md`);
    const existing = this.app.vault.getAbstractFileByPath(notePath);
    if (existing instanceof import_obsidian8.TFile) {
      const existingContent = await this.app.vault.read(existing);
      if (!isNoteEffectivelyEmpty(existingContent)) {
        new import_obsidian8.Notice(`"${noteName}" already has content.`);
        return;
      }
    }
    const fullContext = `This note is being added to an existing series in the folder "${folder.name}". The folder already contains these notes:
${siblingList.join("\n")}

Existing notes content for context:
${contextSnippet}

Write a new note on "${topic}" that fits naturally into this series. Cross-link to existing notes using [[WikiLink]] format where relevant. Do not repeat what's already covered in the existing notes. Add new depth and perspective.`;
    let fullPrompt = mode.prompt.replace(/\{selection\}/g, topic.replace(/"/g, '\\"')).replace(/\{context\}/g, fullContext.replace(/"/g, '\\"')) + getOutputRules();
    if (extraInstructions) {
      fullPrompt += `

Additional instructions from the user:
${extraInstructions}`;
    }
    this.queue.push({
      type: "topic-note",
      noteName,
      newNotePath: notePath,
      fullPrompt,
      mode,
      renameFromContent: true
    });
    const pos = this.queue.length + (this.isProcessing ? 1 : 0);
    new import_obsidian8.Notice(`Queued "${noteName}" (${mode.name}) - position ${pos}. Using ${files.length} existing notes as context.`);
    this.updateStatusBar();
    if (!this.isProcessing) {
      void this.processQueue();
    }
  }
  async expandFolderNotes(direction, mode, folder, extraInstructions) {
    new import_obsidian8.Notice(`Planning new notes for "${folder.name}"... This may take a moment.`);
    logger.info(`Expand folder: direction="${direction.slice(0, 80)}...", mode=${mode.id}, folder=${folder.path}`);
    const files = folder.children.filter(
      (f) => f instanceof import_obsidian8.TFile && f.extension === "md"
    );
    const existingTitles = [];
    let existingSummary = "";
    for (const f of files) {
      const content = await this.app.vault.read(f);
      if (!isNoteEffectivelyEmpty(content)) {
        existingTitles.push(f.basename);
        const firstLines = content.split("\n").slice(0, 30).join("\n");
        existingSummary += `
- "${f.basename}": ${firstLines.slice(0, 300)}`;
      }
    }
    const decompositionGuide = mode.folderDecompositionGuide ? `

DOMAIN-SPECIFIC GUIDANCE for "${mode.name}" mode:
${mode.folderDecompositionGuide}` : "";
    const decompositionPrompt = `You are an expert curriculum designer expanding an existing set of study notes.

The folder "${folder.name}" already contains these notes:
${existingTitles.map((t) => `- "${t}"`).join("\n")}

Brief summaries of existing notes:${existingSummary}

The user wants to ADD NEW notes on the following topic/direction:
"${direction}"

The new notes will be written in the "${mode.name}" style (${mode.description}).${decompositionGuide}

Plan 4-10 NEW notes that complement the existing ones. Do NOT plan notes that duplicate or significantly overlap with existing notes.

Return ONLY valid JSON - no markdown code fences, no explanation, no text before or after. The format:
[
  {"title": "Note Title", "scope": "2-3 sentence description of what this specific note covers in depth", "related": ["Other Note Title"], "order": 1}
]

Rules:
- Every title must be DIFFERENT from all existing notes listed above
- Order from foundational to advanced (use the "order" field, starting at 1)
- Titles should be clear, standalone note titles (3-8 words)
- "related" can reference both NEW notes and EXISTING notes for cross-linking
- Focus specifically on the direction the user requested
- Do not rehash what the existing notes already cover
${extraInstructions ? `
Additional context from the user: ${extraInstructions}` : ""}`;
    try {
      const rawResponse = await this.runClaude(decompositionPrompt);
      let subtopics;
      try {
        const jsonStr = extractJsonArray(rawResponse);
        subtopics = JSON.parse(jsonStr);
      } catch (e) {
        new import_obsidian8.Notice("Failed to parse note plan. Check logs for details.", 8e3);
        logger.error("Expand folder parse error. Raw response: " + rawResponse.slice(0, 500));
        return;
      }
      if (!Array.isArray(subtopics) || subtopics.length === 0) {
        new import_obsidian8.Notice("No new notes were planned. Try rephrasing the direction.", 5e3);
        return;
      }
      subtopics.sort((a, b) => (a.order || 0) - (b.order || 0));
      const existingSet = new Set(existingTitles.map((t) => t.toLowerCase()));
      subtopics = subtopics.filter((s) => {
        const name = sanitizeFilename(s.title).toLowerCase();
        if (existingSet.has(name)) {
          logger.info(`Skipping planned note "${s.title}" - already exists`);
          return false;
        }
        return true;
      });
      if (subtopics.length === 0) {
        new import_obsidian8.Notice("All planned notes already exist in this folder.", 5e3);
        return;
      }
      const allNotes = [
        ...existingTitles.map((t) => `- [[${t}]] (existing)`),
        ...subtopics.map((s) => `- [[${sanitizeFilename(s.title)}]]: ${s.scope} (new)`)
      ].join("\n");
      const directionSnippet = direction.length > 2e3 ? direction.slice(0, 2e3) + "\n...(truncated)" : direction;
      let existingContext = "";
      for (const f of files) {
        const content = await this.app.vault.read(f);
        if (!isNoteEffectivelyEmpty(content)) {
          const snippet = content.length > 1500 ? content.slice(0, 1500) + "\n..." : content;
          existingContext += `

=== ${f.basename} ===
${snippet}`;
        }
      }
      if (existingContext.length > 12e3) {
        existingContext = existingContext.slice(0, 12e3) + "\n...(truncated)";
      }
      for (const sub of subtopics) {
        const subName = sanitizeFilename(sub.title);
        const subPath = (0, import_obsidian8.normalizePath)(`${folder.path}/${subName}.md`);
        const existing = this.app.vault.getAbstractFileByPath(subPath);
        if (existing instanceof import_obsidian8.TFile) {
          const content = await this.app.vault.read(existing);
          if (!isNoteEffectivelyEmpty(content)) {
            logger.info(`Skipping "${subName}" - file already has content`);
            continue;
          }
        }
        const siblingContext = `This note is being added to the folder "${folder.name}" which already has notes on related topics. The user asked to expand with: "${directionSnippet}"

All notes in this series (existing and new):
${allNotes}

Existing notes for context:
${existingContext}

This specific note should focus on: ${sub.scope}

Do NOT repeat what the existing notes already cover. Add new depth, perspective, and content. Cross-link to both existing and new notes using [[WikiLink]] format.`;
        let subPrompt = mode.prompt.replace(/\{selection\}/g, sub.title.replace(/"/g, '\\"')).replace(/\{context\}/g, siblingContext.replace(/"/g, '\\"')) + getOutputRules();
        if (extraInstructions) {
          subPrompt += `

Additional instructions from the user:
${extraInstructions}`;
        }
        const relatedLinks = sub.related.map((r) => `[[${sanitizeFilename(r)}]]`).join(", ");
        subPrompt += `

IMPORTANT: Link to related notes using [[WikiLink]] format. Related: ${relatedLinks}`;
        this.queue.push({
          type: "topic-note",
          noteName: subName,
          newNotePath: subPath,
          fullPrompt: subPrompt,
          mode
        });
      }
      const queued = this.queue.length;
      new import_obsidian8.Notice(`Queued ${queued} new notes to expand "${folder.name}"`);
      this.updateStatusBar();
      if (!this.isProcessing) {
        void this.processQueue();
      }
    } catch (err) {
      const msg = err instanceof Error ? err.message : String(err);
      new import_obsidian8.Notice(`Failed to plan expansion notes: ${msg}`, 8e3);
      logger.error("Expand folder error: " + msg);
    }
  }
  async analyzeExistingNote(file, mode, extraContent) {
    var _a, _b;
    const noteContent = await this.app.vault.read(file);
    if (isNoteEffectivelyEmpty(noteContent)) {
      new import_obsidian8.Notice("This note is empty. Nothing to analyze.");
      return;
    }
    const folderPath = (_b = (_a = file.parent) == null ? void 0 : _a.path) != null ? _b : "";
    const analysisName = sanitizeFilename(`${file.basename} - ${mode.name}`);
    const analysisPath = (0, import_obsidian8.normalizePath)(
      folderPath ? `${folderPath}/${analysisName}.md` : `${analysisName}.md`
    );
    const contentSnippet = noteContent.length > 8e3 ? noteContent.slice(0, 8e3) + "\n...(truncated)" : noteContent;
    let context = contentSnippet;
    if (extraContent) {
      const extraSnippet = extraContent.length > 6e3 ? extraContent.slice(0, 6e3) + "\n...(truncated)" : extraContent;
      context += `

---

ADDITIONAL SOURCE MATERIAL PROVIDED BY USER:

${extraSnippet}`;
    }
    let fullPrompt = mode.prompt.replace(/\{selection\}/g, file.basename.replace(/"/g, '\\"')).replace(/\{context\}/g, context.replace(/"/g, '\\"')) + getOutputRules();
    this.queue.push({
      type: "topic-note",
      noteName: analysisName,
      newNotePath: analysisPath,
      fullPrompt,
      mode
    });
    const pos = this.queue.length + (this.isProcessing ? 1 : 0);
    new import_obsidian8.Notice(`Queued "${analysisName}" - position ${pos}`);
    this.updateStatusBar();
    if (!this.isProcessing) {
      void this.processQueue();
    }
  }
  async analyzeFolderNotes(folder, mode, extraContent) {
    const files = folder.children.filter(
      (f) => f instanceof import_obsidian8.TFile && f.extension === "md"
    );
    if (files.length === 0) {
      new import_obsidian8.Notice("No notes found in this folder.");
      return;
    }
    new import_obsidian8.Notice(`Reading ${files.length} notes and planning analysis...`);
    logger.info(`Folder analysis: ${files.length} notes, mode=${mode.id}, folder=${folder.path}`);
    let combinedContent = "";
    for (const f of files) {
      const content = await this.app.vault.read(f);
      if (!isNoteEffectivelyEmpty(content)) {
        const snippet = content.length > 3e3 ? content.slice(0, 3e3) + "\n..." : content;
        combinedContent += `

=== ${f.basename} ===
${snippet}`;
      }
    }
    if (!combinedContent.trim()) {
      new import_obsidian8.Notice("All notes in this folder are empty.");
      return;
    }
    const combinedSnippet = combinedContent.length > 15e3 ? combinedContent.slice(0, 15e3) + "\n...(truncated)" : combinedContent;
    const decompositionGuide = mode.folderDecompositionGuide ? `

DOMAIN-SPECIFIC GUIDANCE:
${mode.folderDecompositionGuide}` : "";
    let extraSection = "";
    if (extraContent) {
      const extraSnippet = extraContent.length > 6e3 ? extraContent.slice(0, 6e3) + "\n...(truncated)" : extraContent;
      extraSection = `

ADDITIONAL SOURCE MATERIAL PROVIDED BY USER:

${extraSnippet}`;
    }
    const decompositionPrompt = `You are an expert analyst. You have been given existing notes from a folder and your job is to plan an analysis.

EXISTING NOTES IN THE FOLDER:
${combinedSnippet}${extraSection}

The analysis will be written in the "${mode.name}" style (${mode.description}).${decompositionGuide}

Based on the existing content, plan 3-8 analysis notes. Each note should cover one distinct aspect of the analysis.

Return ONLY valid JSON - no markdown code fences, no explanation, no text before or after. The format:
[
  {"title": "Analysis Note Title", "scope": "2-3 sentence description of what this analysis note covers", "related": ["Other Note Title"], "order": 1}
]

Rules:
- Order notes from most critical to least critical
- Titles should be clear and specific to the content being analyzed
- "scope" should describe the specific analysis angle
- "related" should reference other analysis note titles from your list
- Base your analysis plan on what is ACTUALLY in the existing notes, not generic topics
- Do not create generic analysis - be specific to the content`;
    try {
      const rawResponse = await this.runClaude(decompositionPrompt);
      let subtopics;
      try {
        const jsonStr = extractJsonArray(rawResponse);
        subtopics = JSON.parse(jsonStr);
      } catch (e) {
        new import_obsidian8.Notice("Failed to parse analysis plan. Check logs.", 8e3);
        logger.error("Folder analysis parse error. Raw: " + rawResponse.slice(0, 500));
        return;
      }
      if (!Array.isArray(subtopics) || subtopics.length === 0) {
        new import_obsidian8.Notice("No analysis notes planned.", 5e3);
        return;
      }
      subtopics.sort((a, b) => (a.order || 0) - (b.order || 0));
      const subtopicList = subtopics.map((s) => `- [[${sanitizeFilename(s.title)}]]: ${s.scope}`).join("\n");
      for (const sub of subtopics) {
        const subName = sanitizeFilename(sub.title);
        const subPath = (0, import_obsidian8.normalizePath)(`${folder.path}/${subName}.md`);
        const existingSub = this.app.vault.getAbstractFileByPath(subPath);
        if (existingSub instanceof import_obsidian8.TFile) {
          const content = await this.app.vault.read(existingSub);
          if (!isNoteEffectivelyEmpty(content)) {
            logger.info(`Skipping "${subName}" - already has content`);
            continue;
          }
        }
        const siblingContext = `You are analyzing existing notes in the folder "${folder.name}".

EXISTING NOTES CONTENT:
${combinedSnippet}${extraSection}

Other analysis notes in this series:
${subtopicList}

This specific analysis note should focus on: ${sub.scope}`;
        let subPrompt = mode.prompt.replace(/\{selection\}/g, sub.title.replace(/"/g, '\\"')).replace(/\{context\}/g, siblingContext.replace(/"/g, '\\"')) + getOutputRules();
        const relatedLinks = sub.related.map((r) => `[[${sanitizeFilename(r)}]]`).join(", ");
        subPrompt += `

Link to related analysis notes: ${relatedLinks}`;
        this.queue.push({
          type: "topic-note",
          noteName: subName,
          newNotePath: subPath,
          fullPrompt: subPrompt,
          mode
        });
      }
      const total = subtopics.length;
      new import_obsidian8.Notice(`Queued ${total} analysis notes for "${folder.name}"`);
      this.updateStatusBar();
      if (!this.isProcessing) {
        void this.processQueue();
      }
    } catch (err) {
      const msg = err instanceof Error ? err.message : String(err);
      new import_obsidian8.Notice(`Failed to plan analysis: ${msg}`, 8e3);
      logger.error("Folder analysis planning error: " + msg);
    }
  }
  async processQueue() {
    if (this.isProcessing || this.queue.length === 0)
      return;
    this.isProcessing = true;
    logger.info(`Processing queue: ${this.queue.length} item(s)`);
    while (this.queue.length > 0) {
      const item = this.queue.shift();
      this.processingItem = item;
      this.streamData = { currentOutput: "", startTime: Date.now() };
      this.updateStatusBar();
      const label = item.type === "note" || item.type === "topic-note" ? item.noteName : item.type === "append" ? `${item.mode.name} on ${item.fileName}` : item.action.name;
      logger.info(`Processing item: type=${item.type}, label="${label}"`);
      try {
        let content = await this.runClaude(item.fullPrompt);
        content = fixCodeBlocks(content);
        content = fixMermaidBlocks(content);
        content = fixDetailsBlocks(content);
        content = fixCalloutCodeFences(content);
        content = fixDataviewInlineQueries(content);
        content = fixCurrencyDollars(content);
        const elapsed = Date.now() - this.streamData.startTime;
        if (item.type === "note" || item.type === "topic-note") {
          const fmStart = content.indexOf("---");
          if (fmStart > 0) {
            content = content.slice(fmStart);
          }
          const existing = this.app.vault.getAbstractFileByPath(item.newNotePath);
          if (existing instanceof import_obsidian8.TFile) {
            const current = await this.app.vault.read(existing);
            if (isNoteEffectivelyEmpty(current)) {
              await this.app.vault.modify(existing, content);
              new import_obsidian8.Notice(`Filled note: ${item.noteName} (${this.queue.length} remaining)`);
            } else {
              new import_obsidian8.Notice(`"${item.noteName}" already has content, skipped.`);
            }
          } else {
            await this.app.vault.create(item.newNotePath, content);
            new import_obsidian8.Notice(`Created: ${item.noteName} (${this.queue.length} remaining)`);
          }
          if (item.type === "note") {
            this.applyLinkReplacement(item.linkReplacement);
          }
          if (item.type === "topic-note" && item.renameFromContent) {
            const h1 = content.match(/^#\s+(.+)$/m);
            if (h1) {
              const betterName = sanitizeFilename(h1[1].trim());
              if (betterName && betterName !== item.noteName) {
                const dir = item.newNotePath.replace(/[^/]+$/, "");
                const betterPath = (0, import_obsidian8.normalizePath)(`${dir}${betterName}.md`);
                if (!this.app.vault.getAbstractFileByPath(betterPath)) {
                  const created = this.app.vault.getAbstractFileByPath(item.newNotePath);
                  if (created instanceof import_obsidian8.TFile) {
                    await this.app.fileManager.renameFile(created, betterPath);
                    item.noteName = betterName;
                    item.newNotePath = betterPath;
                  }
                }
              }
            }
          }
          this.completedItems.push({ item, chars: content.length, elapsed, timestamp: Date.now() });
        } else if (item.type === "append") {
          const file = this.app.vault.getAbstractFileByPath(item.filePath);
          if (file instanceof import_obsidian8.TFile) {
            const existing = await this.app.vault.read(file);
            await this.app.vault.modify(file, existing + "\n\n" + content);
            new import_obsidian8.Notice(`Appended ${item.mode.name} to ${item.fileName} (${this.queue.length} remaining)`);
            this.completedItems.push({ item, chars: content.length, elapsed, timestamp: Date.now() });
          } else {
            new import_obsidian8.Notice(`File not found: ${item.filePath}`, 5e3);
          }
        } else if (item.type === "inline") {
          if (item.insertionMode === "replace") {
            item.editor.replaceRange(content, item.from, item.to);
            new import_obsidian8.Notice(`Replaced selection with ${item.action.name} result (${this.queue.length} remaining)`);
          } else {
            item.editor.replaceRange("\n\n" + content, item.to, item.to);
            new import_obsidian8.Notice(`Inserted ${item.action.name} result below selection (${this.queue.length} remaining)`);
          }
          this.completedItems.push({ item, chars: content.length, elapsed, timestamp: Date.now() });
        }
      } catch (err) {
        const msg = err instanceof Error ? err.message : String(err);
        const errLabel = this.getItemLabel(item);
        logger.error(`Task failed: "${errLabel}" - ${msg}`);
        this.failedItems.push({ item, error: msg, timestamp: Date.now() });
        new import_obsidian8.Notice(`Failed: ${errLabel} - ${msg}. Use queue viewer to retry.`, 8e3);
      }
    }
    this.processingItem = null;
    this.streamData = { currentOutput: "", startTime: 0 };
    this.isProcessing = false;
    this.updateStatusBar();
    new import_obsidian8.Notice("All queued tasks completed.");
  }
  runClaude(prompt) {
    if (this.settings.aiProvider === "ollama") {
      return this.runOllama(prompt);
    }
    return new Promise((resolve, reject) => {
      const systemPrompt = "Output ONLY the note content as markdown text directly to stdout. Write from your training knowledge. You CAN use web search if needed. Do not ask questions. Do not offer alternatives. Do not attempt to save or write files. Just output the note content directly as your response.";
      let execPath;
      let args;
      let stdinPayload = prompt;
      let codexOutputPath = null;
      if (this.settings.aiProvider === "gemini") {
        execPath = this.settings.geminiPath;
        args = [
          "-p",
          systemPrompt,
          "--approval-mode",
          "plan"
        ];
        if (this.settings.geminiModel) {
          args.push("--model", this.settings.geminiModel);
        }
      } else if (this.settings.aiProvider === "codex") {
        execPath = this.settings.codexPath;
        codexOutputPath = (0, import_obsidian8.normalizePath)(
          `${this.app.vault.configDir}/plugins/${this.manifest.id}/codex-last-message-${Date.now()}.md`
        );
        args = [
          "exec",
          "--sandbox",
          "read-only",
          "--skip-git-repo-check"
        ];
        const adapter = this.app.vault.adapter;
        if (adapter instanceof import_obsidian8.FileSystemAdapter) {
          args.push("--output-last-message", adapter.getFullPath(codexOutputPath));
        } else {
          codexOutputPath = null;
        }
        if (this.settings.codexModel) {
          args.push("--model", this.settings.codexModel);
        }
        args.push("-");
        stdinPayload = systemPrompt + "\n\n" + prompt;
      } else {
        execPath = this.settings.claudePath;
        args = [
          "-p",
          systemPrompt,
          "--max-turns",
          "10",
          "--disallowedTools",
          "Edit,Write,Read,Bash,PowerShell,Glob,Grep"
        ];
        args.push("--model", this.settings.modelFlag || "sonnet");
      }
      const providerLabel = PROVIDER_LABELS[this.settings.aiProvider];
      logger.info(`Spawning [${providerLabel}]: ${execPath} ${args.map((a) => a.length > 40 ? a.slice(0, 40) + "..." : a).join(" ")}`);
      logger.info(`Stdin prompt length: ${prompt.length} chars`);
      const proc = spawnCli(execPath, args);
      let stdout = "";
      let stderr = "";
      proc.stdout.on("data", (data) => {
        const chunk = data.toString();
        stdout += chunk;
        this.streamData.currentOutput = stdout;
      });
      proc.stderr.on("data", (data) => {
        stderr += data.toString();
      });
      const cliNotFoundError = () => new Error(
        `${providerLabel} CLI not found (looked for "${execPath}"). Install it, or set its full path in Settings -> Second Brain Builder. See the README's Troubleshooting section for PATH fixes.`
      );
      proc.on("close", (code) => {
        logger.info(`Process exited with code ${code}, stdout: ${stdout.length} chars, stderr: ${stderr.length} chars`);
        if (code !== 0) {
          const errorDetail = stderr || stdout || `Process exited with code ${code}`;
          logger.error(`Exit code ${code}. Stderr: ${stderr.slice(0, 500)}`);
          logger.error(`Exit code ${code}. Stdout: ${stdout.slice(0, 500)}`);
          const cliMissing = code === 127 || code === 9009 || /is not recognized as an internal or external command|command not found|No such file or directory/i.test(errorDetail);
          reject(cliMissing ? cliNotFoundError() : new Error(errorDetail.trim()));
        } else {
          void (async () => {
            let result = stdout.trim();
            if (codexOutputPath) {
              try {
                const lastMessage = (await this.app.vault.adapter.read(codexOutputPath)).trim();
                if (lastMessage)
                  result = lastMessage;
              } catch (e) {
              }
              try {
                await this.app.vault.adapter.remove(codexOutputPath);
              } catch (e) {
              }
            }
            logger.info(`${providerLabel} responded successfully`);
            resolve(result);
          })();
        }
      });
      proc.on("error", (err) => {
        logger.error(`Spawn error: ${err.message}`);
        reject(err.code === "ENOENT" ? cliNotFoundError() : err);
      });
      proc.stdin.write(stdinPayload);
      proc.stdin.end();
    });
  }
  async runOllama(prompt) {
    var _a;
    const systemPrompt = "Output ONLY the note content as markdown text directly. Write from your training knowledge. Do not ask questions. Do not offer alternatives. Just output the note content directly as your response.";
    const url = `${this.settings.ollamaUrl}/api/generate`;
    const body = JSON.stringify({
      model: this.settings.ollamaModel,
      prompt,
      system: systemPrompt,
      stream: false
    });
    logger.info(`Ollama request: ${url}, model=${this.settings.ollamaModel}, prompt=${prompt.length} chars`);
    let response;
    try {
      response = await (0, import_obsidian8.requestUrl)({
        url,
        method: "POST",
        contentType: "application/json",
        body,
        throw: false
      });
    } catch (err) {
      const msg = err instanceof Error ? err.message : String(err);
      throw new Error(`Ollama connection failed: ${msg}. Is Ollama running at ${this.settings.ollamaUrl}?`);
    }
    if (response.status >= 400) {
      throw new Error(`Ollama error (${response.status}): ${response.text}`);
    }
    const json = response.json;
    if (json.error) {
      throw new Error(`Ollama: ${json.error}`);
    }
    const output = ((_a = json.response) != null ? _a : "").trim();
    this.streamData.currentOutput = output;
    logger.info(`Ollama responded successfully, ${output.length} chars`);
    return output;
  }
};
// Annotate the CommonJS export names for ESM import in node:
0 && (module.exports = {});

/* nosourcemap */