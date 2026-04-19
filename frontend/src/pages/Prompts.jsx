import { useCallback, useEffect, useRef, useState } from 'react'
import { motion, AnimatePresence } from 'framer-motion'
import { api } from '../api/client'
import Button from '../components/Button'

export default function Prompts() {
  const [question, setQuestion]   = useState(null)
  const [answer, setAnswer]       = useState('')
  const [loading, setLoading]     = useState(true)
  const [submitting, setSubmitting] = useState(false)
  const [flash, setFlash]         = useState(false) // "saved" confirmation
  const [error, setError]         = useState('')
  const textareaRef               = useRef(null)

  const loadQuestion = useCallback(async () => {
    setLoading(true)
    setAnswer('')
    setError('')
    try {
      const q = await api.randomQuestion()
      setQuestion(q)
    } catch (e) {
      setError(e.message)
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => { loadQuestion() }, [loadQuestion])

  // Auto-resize textarea
  useEffect(() => {
    const el = textareaRef.current
    if (!el) return
    el.style.height = 'auto'
    el.style.height = el.scrollHeight + 'px'
  }, [answer])

  async function handleSubmit() {
    if (!answer.trim() || !question) return
    setSubmitting(true)
    setError('')
    try {
      await api.answerQuestion(question.id, answer.trim())
      setFlash(true)
      setTimeout(() => {
        setFlash(false)
        loadQuestion()
      }, 800)
    } catch (e) {
      setError(e.message)
    } finally {
      setSubmitting(false)
    }
  }

  return (
    <div className="flex-1 flex flex-col overflow-hidden">
      {/* Header */}
      <div className="px-6 pt-14 pb-4 safe-top shrink-0">
        <h1 className="text-[28px] font-bold tracking-tight2 text-apple-text">Prompts</h1>
        <p className="text-[15px] text-apple-sub mt-0.5">Answer honestly. Skip freely.</p>
      </div>

      <div className="flex-1 flex flex-col px-4 pb-6 overflow-y-auto">
        <AnimatePresence mode="wait">
          {loading ? (
            <motion.div
              key="loading"
              initial={{ opacity: 0 }}
              animate={{ opacity: 1 }}
              exit={{ opacity: 0 }}
              className="flex-1 flex items-center justify-center"
            >
              <div className="w-8 h-8 border-[3px] border-apple-blue border-t-transparent rounded-full animate-spin" />
            </motion.div>
          ) : question ? (
            <motion.div
              key={question.id}
              initial={{ opacity: 0, y: 24 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -16 }}
              transition={{ duration: 0.3, ease: [0.25, 0.46, 0.45, 0.94] }}
              className="flex flex-col gap-4"
            >
              {/* Question card */}
              <div className="bg-white rounded-3xl shadow-apple-sm px-6 py-6">
                <p className="text-[19px] font-semibold text-apple-text leading-snug">
                  {question.text}
                </p>
              </div>

              {/* Answer input */}
              <div className="bg-white rounded-3xl shadow-apple-sm px-5 py-4">
                <textarea
                  ref={textareaRef}
                  value={answer}
                  onChange={e => setAnswer(e.target.value)}
                  placeholder="Type your answer…"
                  rows={3}
                  className={[
                    'w-full resize-none bg-transparent',
                    'text-[16px] text-apple-text placeholder:text-apple-sub/50',
                    'outline-none leading-relaxed',
                  ].join(' ')}
                />
              </div>

              {error && (
                <p className="text-apple-red text-[13px] px-1">{error}</p>
              )}

              {/* Actions */}
              <div className="flex flex-col gap-2 mt-1">
                <Button
                  fullWidth
                  onClick={handleSubmit}
                  disabled={!answer.trim() || submitting || flash}
                >
                  {flash ? 'Saved ✓' : submitting ? 'Saving…' : 'Submit'}
                </Button>
                <Button
                  fullWidth
                  variant="ghost"
                  onClick={loadQuestion}
                  disabled={submitting || flash}
                >
                  Skip this question
                </Button>
              </div>
            </motion.div>
          ) : null}
        </AnimatePresence>
      </div>
    </div>
  )
}
