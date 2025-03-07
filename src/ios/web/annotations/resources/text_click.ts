// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Handle tap on 'CHROME_ANNOTATION' elements.
 */

import {LiveTaskTimer, TaskTimer} from '//ios/web/annotations/resources/text_tasks.js';

// Consumer of CHROME_ANNOTATION `HTMLElement` taps.
type AnnotationsTapConsumer = {
  (element: HTMLElement, cancel: boolean): void;
};

// Delay while checking for DOM mutations.
const DOM_MUTATION_DELAY_MS = 300;

// Monitors DOM mutations between instance construction until a call to
// `stopObserving`.
class MutationsTracker {
  // Returns true if DOM mutations occurred.
  public hasMutations = false;

  private mutationObserver: MutationObserver;
  private mutationExtendId = 0;

  // Mutation observer for handling added and removed nodes.
  private mutationCallback = (mutationList: MutationRecord[]) => {
    for (let mutation of mutationList) {
      if (mutation.target.contains(this.initialEvent.target as Node)) {
        this.hasMutations = true;
        this.mutationObserver?.disconnect();
        return;
      }
    }
  };

  // Constructs a new instance given an `initialEvent` and starts listening for
  // changes to the DOM. `initialEvent` is the click event on a
  // CHROME_ANNOTATION at the beginning of the capture phase; it is used in
  // `hasPreventativeActivity` ta make sure the bubbling event received is the
  // same. If not, it is considered, like mutations, as a preventative activity.
  constructor(
      private readonly initialEvent: Event, root: Element,
      private taskTimer: TaskTimer = new LiveTaskTimer()) {
    this.mutationObserver = new MutationObserver(this.mutationCallback);
    this.mutationObserver.observe(
        root, {attributes: false, childList: true, subtree: true});
  }

  // Returns true if event doesn't match the event passed at construction, or it
  // was prevented or if any DOM mutations occurred.
  hasPreventativeActivity(event: Event): boolean {
    return event !== this.initialEvent || event.defaultPrevented ||
        this.hasMutations;
  }

  // Extends DOM observation by triggering `then` after `delayMs`. This can
  // be called multiple times if needed.
  extendObservation(then: Function, delayMs: number): void {
    if (this.mutationExtendId) {
      this.taskTimer.clear(this.mutationExtendId);
    }
    this.mutationExtendId = this.taskTimer.reset(then, delayMs);
  }

  stopObserving(): void {
    if (this.mutationExtendId) {
      this.taskTimer.clear(this.mutationExtendId);
    }
    this.mutationExtendId = 0;
    this.mutationObserver?.disconnect();
  }

  // Force updating before next js main thread cycle.
  updateForTesting(): void {
    this.mutationCallback(this.mutationObserver.takeRecords());
  }
}

// Class to monitor taps on CHROME_ANNOTATION elements.
class TextClick {
  private mutationObserver: MutationsTracker|null = null;

  constructor(
      private root: Element, private consumer: AnnotationsTapConsumer,
      private taskTimer: TaskTimer = new LiveTaskTimer(),
      private mutationCheckDelay = DOM_MUTATION_DELAY_MS) {}

  // Starts event listeners.
  start(): void {
    // First check when capturing down event.
    this.root.addEventListener('click', this.onClick, {capture: true});
    // Last checks when bubbling up event.
    this.root.addEventListener('click', this.onClick);
  }

  // Stops event listeners.
  stop(): void {
    this.root.removeEventListener('click', this.onClick, {capture: true});
    this.root.removeEventListener('click', this.onClick);
    this.cancelObserver();
  }

  // Force updating before next js main thread cycle.
  updateForTesting(): void {
    this.mutationObserver?.updateForTesting();
  }

  // Callback for tap handler.
  private onClick = (event: Event) => {
    this.handleTopTap(event);
  };

  // Stops observing DOM mutations.
  private cancelObserver(): void {
    this.mutationObserver?.stopObserving();
    this.mutationObserver = null;
  }

  // Monitors taps at the top, document level. This checks if it is tap
  // triggered by an annotation and if no DOM mutation have happened while the
  // event is bubbling up. If it's the case, the annotation callback is called.
  private handleTopTap(event: Event): void {
    const annotation = event.target;
    if (annotation instanceof HTMLElement &&
        annotation.tagName === 'CHROME_ANNOTATION') {
      if (event.eventPhase === Event.CAPTURING_PHASE) {
        // Initiates a `mutationObserver` that will be checked at bubble up
        // phase where it will be decided if the click should be cancelled.
        this.cancelObserver();
        this.mutationObserver =
            new MutationsTracker(event, this.root, this.taskTimer);
      } else if (this.mutationObserver) {
        // At BUBBLING_PHASE.
        if (this.mutationObserver.hasPreventativeActivity(event)) {
          this.consumer(annotation, /*cancel*/ true);
          this.cancelObserver();
        } else {
          this.mutationObserver.extendObservation(() => {
            if (this.mutationObserver) {
              this.consumer(annotation, this.mutationObserver.hasMutations);
              this.cancelObserver();
            }
          }, this.mutationCheckDelay);
        }
      }
    } else {
      this.cancelObserver();
    }
  }
}

export {
  DOM_MUTATION_DELAY_MS,
  AnnotationsTapConsumer,
  TextClick,
}
